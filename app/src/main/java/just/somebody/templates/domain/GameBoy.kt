package just.somebody.templates.domain

import android.annotation.SuppressLint
import android.net.Uri
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.presentation.widgets.GameBoySpeaker
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking // For synchronous file loading
import kotlinx.coroutines.withContext
import androidx.documentfile.provider.DocumentFile // Needed for DocumentFile operations
import just.somebody.templates.appModule.storage.ExternalStorageManager

enum class Buttons {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  A,
  B,
  SELECT,
  START,
}

class GameBoy
{

  private var currentRomUri: String? = null

  fun loadROM(ROM: ByteArray, ROM_URI: String)
  {
    this.currentRomUri = ROM_URI
    updateStaticRomUri(ROM_URI)
    App.appModule.linkCable.onByteReceived = { byte -> nativeRecieveByte(byte.toByte()) }
    nativeLoadROM(ROM, ROM.size)
  }

  fun startEmulator()   { nativeStartEmulator() }
  fun stopEmulator()    { nativeStopEmulator() }
  fun resumeEmulator()  { nativeResumeEmulator() }
  fun pauseEmulator()   { nativePauseEmulator() }

  // - - - Input
  fun sendButton(
    BUTTON: Buttons,
    IS_PRESSED: Boolean
  ) {
    nativeSetButtonState(BUTTON.ordinal, IS_PRESSED)
  }

  // - - - Native Bindings for Emulator Core (existing) - - -

  private external fun nativeLoadROM(ROM: ByteArray, SIZE: Int)
  private external fun nativeSetButtonState(BUTTON: Int, PRESSED: Boolean)
  private external fun nativeStartEmulator()
  private external fun nativeStopEmulator()
  private external fun nativePauseEmulator()
  private external fun nativeResumeEmulator()

  // - - - Native Bindings for OpenGL ES Rendering (existing)
  external fun nativeOnSurfaceCreated()
  external fun nativeOnSurfaceChanged(width: Int, height: Int)
  external fun nativeOnDrawFrame()
  external fun nativeRecieveByte(BYTE : Byte)

  // - - - Static method for C++ to call back to request a render
  companion object
  {
    init { System.loadLibrary("native-lib") }

    // - - - Reference to the GLSurfaceView instance to call requestRender()
    @SuppressLint("StaticFieldLeak")
    private var glSurfaceViewInstance: android.opengl.GLSurfaceView? = null
    private val speaker = GameBoySpeaker()

    @Volatile // Ensure visibility across threads
    private var staticCurrentRomUri: String? = null

    @JvmStatic
    fun setGLSurfaceView(view: android.opengl.GLSurfaceView)
    { glSurfaceViewInstance = view }

    @JvmStatic
    fun sendByte(BYTE: Byte)
    { App.appModule.linkCable.sendByte(BYTE.toInt())}

    // - - - Function to update the static ROM URI from a GameBoy instance
    @JvmStatic
    internal fun updateStaticRomUri(romUri: String?)
    {
      staticCurrentRomUri = romUri
      ForgeLogger.info("Static current ROM URI updated to: $romUri")
    }


    @JvmStatic
    fun requestRenderFromNative()
    {
      glSurfaceViewInstance?.queueEvent { glSurfaceViewInstance?.requestRender() }
    }

    @JvmStatic
    fun nativePlayAudio(SAMPLE_BUFFER : ByteArray)
    { speaker.play(SAMPLE_BUFFER)  }

    @JvmStatic
    fun nativeStopAudio()
    { /*ignore*/ }

    private suspend fun getGameSaveFileUri(): Uri?
    {
      val romUriString = staticCurrentRomUri
      if (romUriString == null)
      {
        ForgeLogger.error("Kotlin: getGameSaveFileUri: No ROM URI is currently loaded.")
        return null
      }

      return withContext(Dispatchers.IO)
      {
        val romUri  = Uri.parse(romUriString)
        val context = App.appModule.context

        val romDocumentFile = DocumentFile.fromSingleUri(context, romUri)
        if (romDocumentFile == null || !romDocumentFile.exists() || !romDocumentFile.isFile)
        {
          ForgeLogger.error("Kotlin: ROM DocumentFile not found or is invalid for URI: $romUriString")
          return@withContext null
        }

        val pathSegments  = romUri.pathSegments
        val documentIndex = pathSegments.indexOf("document")

        if (documentIndex == -1 || documentIndex == 0)
        {
          val parent = romDocumentFile.parentFile
          if (parent == null || !parent.isDirectory)
          {
            ForgeLogger.error("Kotlin: Could not determine parent directory from single ROM URI (not a tree URI or parent null): $romUriString")
            return@withContext null
          }
          ForgeLogger.warn("Kotlin: ROM URI is not a standard tree/document URI. Using romDocumentFile.parentFile. URI: $romUriString")
          return@withContext resolveSaveFileInParent(parent, romDocumentFile)
        }

        val treeUriPart       = romUri.toString().substringBefore("/document/")
        val rootTreeUri       = Uri.parse(treeUriPart)
        val rootDocumentFile  = DocumentFile.fromTreeUri(context, rootTreeUri)

        if (rootDocumentFile == null || !rootDocumentFile.isDirectory)
        {
          ForgeLogger.error("Kotlin: Could not resolve root DocumentFile from tree URI part: $treeUriPart")
          return@withContext null
        }

        // - - - Reconstruct the relative path of the ROM from the root of the tree
        val romPathInTree = romUri.toString().substringAfterLast("/document/")
        val pathSegmentsInTree = romPathInTree.split('/')

        // - - - Traverse from the root DocumentFile to the ROM's parent directory
        var currentParentDir: DocumentFile? = rootDocumentFile
        for (i in 0 until pathSegmentsInTree.size - 1)
        {
          val segment = pathSegmentsInTree[i]
          if (segment.isEmpty()) continue
          currentParentDir = currentParentDir?.findFile(segment)
          if (currentParentDir == null || !currentParentDir.isDirectory)
          {
            ForgeLogger.error("Kotlin: Failed to traverse to ROM parent directory segment '$segment' for URI: $romUriString")
            return@withContext null
          }
        }
        val romParentDirectory = currentParentDir

        if (romParentDirectory == null || !romParentDirectory.isDirectory)
        {
          ForgeLogger.error("Kotlin: Final ROM parent directory is null or not a directory for URI: $romUriString")
          return@withContext null
        }

        return@withContext resolveSaveFileInParent(romParentDirectory, romDocumentFile)
      }
    }

    private suspend fun resolveSaveFileInParent(ROM_PARENT_DIRECTORY: DocumentFile, ROM_DOCUMENT_FILE: DocumentFile): Uri?
    {
      var savesDirectory = ROM_PARENT_DIRECTORY.findFile("saves")
      if (savesDirectory == null || !savesDirectory.isDirectory)
      {
        savesDirectory = ROM_PARENT_DIRECTORY.createDirectory("saves")
        if (savesDirectory == null)
        {
          ForgeLogger.error("Kotlin: Failed to create 'saves' directory in ${ROM_PARENT_DIRECTORY.uri}")
          return null
        }
        ForgeLogger.info("Kotlin: Created 'saves' directory: ${savesDirectory.uri}")
      }

      val romFileName   = ROM_DOCUMENT_FILE.name ?: "unknown_rom.gb"
      val saveFileName = romFileName.replace(".gb", ".sav", ignoreCase = true)

      var saveFile = savesDirectory.findFile(saveFileName)
      if (saveFile == null)
      {
        saveFile = savesDirectory.createFile(ExternalStorageManager.MIME_BINARY, saveFileName)
        if (saveFile == null)
        {
          ForgeLogger.error("Kotlin: Failed to create save file '$saveFileName' in ${savesDirectory.uri}")
          return null
        }
        ForgeLogger.info("Kotlin: Created new save file: ${saveFile.uri}")
      }
      return saveFile.uri
    }


    @JvmStatic
    fun saveRamToFile(RAM_DATA: ByteArray, RAM_SIZE: Int): Boolean
    {
      if (RAM_DATA.size != RAM_SIZE)
      {
        ForgeLogger.error("Kotlin: saveRamToFile: Mismatch in ramData.size (${RAM_DATA.size}) and ramSize ($RAM_SIZE)")
        return false
      }

      ForgeLogger.info("Kotlin: Attempting to save RAM (size: $RAM_SIZE)")

      CoroutineScope(Dispatchers.IO).launch()
      {
        val saveFileUri = getGameSaveFileUri()
        if (saveFileUri != null)
        {
          val success = App.appModule.externalStorageManager.saveFileFromUri(saveFileUri, RAM_DATA)
          if (success)  ForgeLogger.info("Kotlin: Successfully saved RAM to $saveFileUri.")
          else          ForgeLogger.error("Kotlin: Failed to save RAM to $saveFileUri.")
        }
        else ForgeLogger.error("Kotlin: Could not resolve save file URI. Save failed.")
      }
      return true
    }

    @JvmStatic
    fun loadRamFromFile(RAM_DATA_BUFFER: ByteArray, BUFFER_SIZE: Int): Boolean
    {
      if (RAM_DATA_BUFFER.size != BUFFER_SIZE)
      {
        ForgeLogger.error("Kotlin: loadRamFromFile: Mismatch in ramDataBuffer.size (${RAM_DATA_BUFFER.size}) and bufferSize ($BUFFER_SIZE)")
        RAM_DATA_BUFFER.fill(0)
        return false
      }

      ForgeLogger.info("Kotlin: Attempting to load RAM (expected size: $BUFFER_SIZE)")

      return runBlocking(Dispatchers.IO) {
        val saveFileUri = getGameSaveFileUri()
        if (saveFileUri != null)
        {
          val loadedData = App.appModule.externalStorageManager.readFileFromUri(saveFileUri)
          if (loadedData != null)
          {
            if (loadedData.size == BUFFER_SIZE)
            {
              loadedData.copyInto(RAM_DATA_BUFFER)
              ForgeLogger.info("Kotlin: Successfully loaded ${loadedData.size} bytes from $saveFileUri.")
              true
            }
            else
            {
              ForgeLogger.error("Kotlin: Loaded data size mismatch for $saveFileUri (expected: $BUFFER_SIZE, actual: ${loadedData.size}). Clearing RAM.")
              RAM_DATA_BUFFER.fill(0)
              false
            }
          }
          else
          {
            ForgeLogger.info("Kotlin: No save file found or failed to read for $saveFileUri. Starting with fresh RAM.")
            RAM_DATA_BUFFER.fill(0)
            false
          }
        }
        else
        {
          ForgeLogger.error("Kotlin: Could not resolve save file URI. Load failed.")
          RAM_DATA_BUFFER.fill(0)
          false
        }
      }
    }


    @JvmStatic
    fun getExpectedSaveSize(): Int
    {
      ForgeLogger.warn("Kotlin: getExpectedSaveSize called. Returning 0 as size is managed by C++ cartridge core.")
      return 0
    }
  }
}
