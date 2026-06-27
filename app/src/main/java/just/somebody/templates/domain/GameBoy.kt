package just.somebody.templates.domain

import android.annotation.SuppressLint
import android.net.Uri
import androidx.annotation.Keep
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.presentation.widgets.GameBoySpeaker
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking // For synchronous file loading
import kotlinx.coroutines.withContext
import androidx.documentfile.provider.DocumentFile // Needed for DocumentFile operations
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.data.entities.AchievementEntity
import just.somebody.templates.domain.models.Palette
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import java.util.Collections
import java.util.concurrent.ConcurrentHashMap

/**
 * Different ways a pause can be triggered
 * @property SETTINGS : Paused by opening the settings panel
 * @property FOCUS : The app lost focus
 * @property IO : The app has to do some io
 */
enum class PauseTrigger
{
  SETTINGS,
  FOCUS,
  IO
}

/** The game boy buttons */
enum class Buttons
{
  RIGHT, LEFT, UP, DOWN,
  A, B,
  SELECT, START,
}


/**
 * Game Boy : the kotlin / c++ interface the tie between the c emulator and the kotlin app
 * @property currentRomUri (String?) : the uri of the currently playing game
 * @property pauseTriggers (Set) : the pause triggers
 * @property sessionUnlockedTitles (Set) : the set of all titles unlocked during the play
 * @property deferredRamData (ByteArray?) : the binary ram data to be saved
 * @property isRamDirty (Boolean) : dirty ram flag to save (deferred saving)
 */
@Keep
class GameBoy
{
  // - - - State - - -

  private var currentRomUri: String? = null
  private val pauseTriggers = Collections.newSetFromMap(ConcurrentHashMap<PauseTrigger, Boolean>())

  // - - - Cache of unlocked achievement titles for the current session/game to prevent duplicate notifications
  private val sessionUnlockedTitles = Collections.synchronizedSet(mutableSetOf<String>())

  // - - - Deferred RAM saving state
  private var deferredRamData : ByteArray? = null
  private var isRamDirty       : Boolean    = false


  // - - - Pause / Resume emulator - - -

  /**
   * Kotlin side interface to pause the emulator, can be unlocked only by the same trigger
   * @param TRIGGER (PauseTrigger) : what paused the emulator
   */
  fun pauseEmulator(TRIGGER: PauseTrigger)
  {
    if (pauseTriggers.isEmpty())
    {
      nativePauseEmulator()
      ForgeLogger.info("Emulator paused by $TRIGGER")
    }
    pauseTriggers.add(TRIGGER)
  }

  /**
   * Kotlin side interface to resume the emulator, works only if the pause trigger and resume trigger match
   * @param TRIGGER (PauseTrigger) : the reason for resuming
   */
  fun resumeEmulator(TRIGGER: PauseTrigger)
  {
    pauseTriggers.remove(TRIGGER)
    if (pauseTriggers.isEmpty())
    {
      nativeResumeEmulator()
      ForgeLogger.info("Emulator resumed (last trigger cleared: $TRIGGER)")
    }
    else
    { ForgeLogger.info("Emulator still paused. Remaining triggers: $pauseTriggers") }
  }

  /** Native function to pause emualator */
  private external fun nativePauseEmulator()

  /** Native function to resume emulator */
  private external fun nativeResumeEmulator()


  // - - - Stop / Start emulator - - -

  /** Kotlin side interface to start the emulator */
  fun startEmulator()
  {
    pauseTriggers.clear()
    isRamDirty      = false
    deferredRamData = null
    nativeStartEmulator()
  }

  /** Kotlin side interface to stop the emulator */
  fun stopEmulator()
  {
    pauseTriggers.clear()
    flushRam()
    nativeStopEmulator()
  }

  /** Native function to start emulator */
  private external fun nativeStartEmulator()

  /** Native function to stop emulator */
  private external fun nativeStopEmulator()


  // - - - Set Volumes - - -

  /**
   * Kotlin side interface to set channel volumes
   * @param VOLUMES (FloatArray) : the volumes of each channel
   */
  fun setVolumes(VOLUMES: FloatArray) { nativeSetVolumes(VOLUMES) }

  /**
   * Native function to set volume
   * @param VOLUMES (FloatArray) : the volumes of the channels
   */
  private external fun nativeSetVolumes(VOLUMES: FloatArray)


  // - - - Save File Management - - -

  /** Deletes the ram file */
  fun deleteRamFile()
  {
    runBlocking(Dispatchers.IO)
    {
      val uri = getGameSaveFileUri()
      if (uri == null)
      {
        SnackbarController.sendEvent(SnackbarEvent("There is no save file to delete"))
        return@runBlocking
      }
      if (App.appModule.externalStorageManager.deleteFileFromUri(uri))
      {
        SnackbarController.sendEvent(SnackbarEvent("Deleted save file"))
      }
      else
      {
        SnackbarController.sendEvent(SnackbarEvent("Failed to delete save file"))
      }
    }
  }

  /**
   * Deletes the save file
   * @param ROM_URI (String) : the game whose save is to be delete
   */
  fun deleteRamFile(ROM_URI : String)
  {
    this.currentRomUri = ROM_URI
    updateStaticRomUri(ROM_URI)
    deleteRamFile()
  }

  /** Kotlin side interface to flush the save file */
  fun flushSave()       { nativeFlushSave() }

  /** Native function to flush a save */
  private external fun nativeFlushSave()


  // - - - Save State Management - - -

  /**
   * Save state kotlin side view
   * @return (ByteArray?) : the save state binary if successfully created
   */
  fun saveState(): ByteArray? = nativeSaveState()

  /**
   * Kotlin side view of the save state load
   * @param DATA (ByteArray) : the state binary
   * @return (Boolean) : whether the state was loaded correctly
   */
  fun loadState(DATA: ByteArray): Boolean = nativeLoadState(DATA, DATA.size)

  /**
   * Native call to make a save state
   * @return (ByteArray?) the binary save state if it got created
   */
  private external fun nativeSaveState(): ByteArray?

  /**
   * Native call to load a save state
   * @param DATA (ByteArray) the save state binary
   * @param SIZE (Int) : the size of the save state
   * @return (Boolean) : whether the save state loaded correctly
   */
  private external fun nativeLoadState(DATA: ByteArray, SIZE: Int): Boolean

  /**
   * Kotlin side interface for loading a rom to the emulator
   * @param ROM (ByteArray) : the rom binary
   * @param ROM_URI (String) : the location of the rom binary
   */


  // - - - Loading a Game - - -

  fun loadROM(ROM: ByteArray, ROM_URI: String)
  {
    resetActivityFlag()
    this.currentRomUri = ROM_URI
    updateStaticRomUri(ROM_URI)
    
    // - - - Clear session cache and pre-load unlocked achievements from DB
    sessionUnlockedTitles.clear()
    App.appModule.mainScope.launch(Dispatchers.IO)
    {
      val game = App.appModule.repo.getGameByUri(ROM_URI)
      if (game != null)
      {
        val unlocked = App
          .appModule
          .database
          .achievementDAO()
          .getAchievementsForGameOnce(game.id)
        sessionUnlockedTitles.addAll(unlocked.map { it.title })
        ForgeLogger.info("RA: Pre-loaded ${unlocked.size} unlocked achievements for duplicate filtering.")
      }
    }

    nativeLoadROM(ROM, ROM.size)
  }

  /**
   * Native function to load the rom into memory
   * @param ROM (ByteArray) : the rom binary
   * @param SIZE (Int) : the size of the rom
   */
  private external fun nativeLoadROM(ROM: ByteArray, SIZE: Int)


  // - - - Visuals - - -

  /**
   * Kotlin side interface to set DMG color palette
   * @param PALETTE (Palette) : the 4 color palette
   */
  fun setPalette(PALETTE: Palette)
  {
    val colors = PALETTE.colors.map { android.graphics.Color.parseColor(it) }.toIntArray()
    val packedColors = colors.map()
    { argb: Int ->
      val r = (argb shr 16) and 0xFF
      val g = (argb shr 8) and 0xFF
      val b = argb and 0xFF
      val a = (argb shr 24) and 0xFF
      (r) or (g shl 8) or (b shl 16) or (a shl 24)
    }.toIntArray()
    nativeChangePalette(packedColors)
  }

  /**
   * Kotlin side interface to set shader
   * @param INDEX : The shader index
   */
  fun setShader(INDEX : Int)  { nativeChangeShader(INDEX) }

  /**
   * Native palette change function
   * @param COLORS (IntArray) : the dmg palette to use
   */
  private external fun nativeChangePalette(COLORS : IntArray)

  /**
   * Native shader change function
   * @param INDEX (Int) : the shader index
   */
  private external fun nativeChangeShader(INDEX: Int)


  // - - - Rendering - - -

  /**
   * Captures the frame of the screen
   * @return (IntArray?) : the image if created
   */
  external fun nativeCaptureFrame(): IntArray?

  /** Native call to tie opengl with emulator */
  external fun nativeOnSurfaceCreated()

  /**
   * Native call to update of the gl view
   * @param WIDTH (Int) : the width of the new screen
   * @param HEIGHT (Int) : the height of the new screen
   */
  external fun nativeOnSurfaceChanged(WIDTH: Int, HEIGHT: Int)

  /** Native call to draw frame */
  external fun nativeOnDrawFrame()


  // - - - Input - - -

  /**
   * Kotlin side interface to set fast forward
   * @param ENABLED (Boolean) : whether to enable fast forward
   */
  fun setFastForward(ENABLED : Boolean) { nativeSetFastForward(ENABLED) }

  /**
   * Native function to set fast forward
   * @param ENABLED (Boolean) : whether to enable fast forward
   */
  private external fun nativeSetFastForward(ENABLED : Boolean)

  /**
   * Kotlin side interface for setting a game boy button
   * @param BUTTON (Buttons) : the button to be updated
   * @param IS_PRESSED (Boolean) : whether the button is pressed or not
   */
  fun sendButton(
    BUTTON     : Buttons,
    IS_PRESSED : Boolean)
  { nativeSetButtonState(BUTTON.ordinal, IS_PRESSED) }

  /**
   * Native function to set game boy buttons
   * @param BUTTON (Int) : the button to be updated
   * @parma PRESSED (Boolean) : whether the button is pressed or not
   */
  private external fun nativeSetButtonState(BUTTON: Int, PRESSED: Boolean)


  // - - - Retro Achievements - - -

  /**
   * Kotlin side interface to notify the emulator of an HTTP response
   * @param BODY (String) : The response body
   * @param STATUS (Int) : The HTTP status code
   * @param CALLBACK_PTR (Long) : The native callback pointer
   */
  fun notifyHttpResponse(BODY: String, STATUS: Int, CALLBACK_PTR: Long)
  { nativeNotifyHttpResponse(BODY, STATUS, CALLBACK_PTR) }

  /**
   * Kotlin side interface to log to Retro Achievements using password
   * @param USERNAME (String) : the user name
   * @param PASSWORD (String) : the password
   */
  fun raLoginWithPassword(USERNAME: String, PASSWORD: String) { nativeRaLoginWithPassword(USERNAME, PASSWORD) }

  /**
   * Kotlin side interface to log to Retro Achievements using token
   * @param USERNAME (String) : the user name
   * @param TOKEN (String) : the app login token
   */
  fun raLoginWithToken(USERNAME: String, TOKEN: String) { nativeRaLoginWithToken(USERNAME, TOKEN) }

  /** Kotlin side interface for loging out of retro achievements */
  fun raLogout() { nativeRaLogout() }

  /**
   * Kotlin side interface for setting hardcore mode
   * @param ENABLED (Boolean) : whether hardcore mode is enabled
   */
  fun raSetHardcoreMode(ENABLED: Boolean) { nativeRaSetHardcoreMode(ENABLED) }

  /**
   * Native login function with password
   * @param USERNAME (String) : the user name
   * @param PASSWORD (String) : the RA password
   */
  private external fun nativeRaLoginWithPassword(USERNAME: String, PASSWORD: String)

  /**
   * Native login function with token
   * @param USERNAME (String) : the user name
   * @param TOKEN (String) : the login token
   */
  private external fun nativeRaLoginWithToken(USERNAME: String, TOKEN: String)

  /** Native Retro achievement logout */
  private external fun nativeRaLogout()

  /**
   * Native call to set hardcore mode (no save states)
   * @param ENABLED (Boolean) : whether the hardcore mode is enabled
   */
  private external fun nativeRaSetHardcoreMode(ENABLED: Boolean)

  /**
   * Native callback to an http response
   * @param BODY (String) : The body of the http response
   * @param STATUS (Int) : the status code of the http response
   * @param CALLBACK_PTR (Long) : a pointer to the callback
   */
  external fun nativeNotifyHttpResponse(BODY: String, STATUS: Int, CALLBACK_PTR: Long)

  /**
   * Handles achievement unlock events from the native core
   * @param ID (Int) : the id of the achievement
   * @param TITLE (String) : the title of the achievement
   * @param DESCRIPTION (String) : the description of the achievements
   * @param POINTS (Int) : the points earnt by this achievement
   * @param BADGE_URL (String) : the url of the badge to be downloaded
   * @param IS_HARDCORE (Boolean) : whether the achievement was unlocked in hardcore mode (no save states)
   * @param TIMESTAMP (Long) : when the achievement was unlcoked
   * @param IS_SILENT (Boolean) : whether to trigger a notification
   */
  fun onAchievementUnlocked(
    ID          : Int,
    TITLE       : String,
    DESCRIPTION : String,
    POINTS      : Int,
    BADGE_URL   : String,
    IS_HARDCORE : Boolean,
    TIMESTAMP   : Long,
    IS_SILENT   : Boolean)
  {
    // - - - Filter out RetroAchievements emulator warnings
    if (TITLE.contains("Warning", ignoreCase = true) && 
        TITLE.contains("Emulator", ignoreCase = true) && 
        DESCRIPTION.contains("Hardcore", ignoreCase = true)) { return }

    // - - - Duplicate check: If we already have this title in our "unlocked" set, don't show a notification
    val isAlreadyUnlocked = sessionUnlockedTitles.contains(TITLE)

    App.appModule.mainScope.launch()
    {
      val dao   = App.appModule.database.achievementDAO()
      val game  = App.appModule.repo.getGameByUri(staticCurrentRomUri ?: "")
      
      if (game != null)
      {
        val achievement = AchievementEntity(
          raId        = ID,
          gameId      = game.id,
          title       = TITLE,
          description = DESCRIPTION,
          points      = POINTS,
          badgeUrl    = BADGE_URL,
          unlockDate  =
            if (TIMESTAMP > 0)  TIMESTAMP
            else                System.currentTimeMillis(),
          isHardcore  = IS_HARDCORE)
        dao.insertAchievement(achievement)
        sessionUnlockedTitles.add(TITLE)
      }

      // - - - Show Toast and Notification only if it's NOT a duplicate and NOT a silent sync
      if (!IS_SILENT && !isAlreadyUnlocked)
      {
        ForgeLogger.info("RA: Showing notification for achievement: $TITLE")
        SnackbarController.sendEvent(SnackbarEvent("Achievement Unlocked: $TITLE"))
        App.appModule.notificationManager.showNotification(
          CONTEXT         = App.appModule.context,
          CHANNEL_ID      = "ACHIEVEMENTS",
          NOTIFICATION_ID = ID,
          TITLE           = "Achievement Unlocked!",
          MESSAGE         = "$TITLE ($POINTS pts)",
          ICON_RES        = R.drawable.trophy)
      }
      else if (!IS_SILENT)
      {
        ForgeLogger.info("RA: Skipping duplicate notification for: $TITLE")
      }
    }
  }


  /**
   * Static companion object
   * @param glSurfaceViewInstance (GLSurfaceView) : the open gl surface to show the emulator screen
   * @param speaker (GameBoySpeaker) : the audio system to be tied to emulator
   * @param staticCurrentRomUri (String?) : the current game rom location
   * @param onFirstActivity (Lambda) : setting the game up lambda
   * @param activityDetected (Boolean) : whether some activity was detected
   */
  companion object
  {
    /** Loads the c emulator core */
    init { System.loadLibrary("PocketPixel") }

    // - - - Reference to the GLSurfaceView instance to call requestRender()
    @SuppressLint("StaticFieldLeak")
    private var glSurfaceViewInstance: android.opengl.GLSurfaceView? = null

    private val speaker = GameBoySpeaker()

    @Volatile
    private var staticCurrentRomUri: String? = null

    var onFirstActivity: (() -> Unit)? = null

    @Volatile
    private var activityDetected = false

    /**
     * Ties the surface view to the emulator screen
     * @param VIEW (GLSurfaceView) : the surface view to be tied to the emulator
     */
    @JvmStatic
    fun setGLSurfaceView(VIEW: android.opengl.GLSurfaceView)
    { glSurfaceViewInstance = VIEW }

    /** resets activity flags */
    fun resetActivityFlag()
    {
      activityDetected = false
      ForgeLogger.info("Core activity flag reset.")
    }

    /**
     * Sends a byte to the server
     * @param BYTE (Byte) : the byte to be sent
     */
    @Deprecated("No longer needed, cant do multiplayer")
    @JvmStatic
    fun sendByte(BYTE: Byte)
    {
      val sb : Int = BYTE.toInt() and 0xFF
    }

    /**
     * Function to update the static ROM URI from a GameBoy instance
     * @param ROM_URI (String?) : the uri to be updated
     */
    @JvmStatic
    internal fun updateStaticRomUri(ROM_URI: String?)
    {
      staticCurrentRomUri = ROM_URI
      ForgeLogger.info("Static current ROM URI updated to: $ROM_URI")
    }

    /** Asks the core to render a frame */
    @JvmStatic
    fun requestRenderFromNative()
    {
      if (!activityDetected)
      {
        activityDetected = true
        onFirstActivity?.invoke()
      }
      glSurfaceViewInstance?.requestRender()
    }


    // - - - Achievements - - -

    /**
     * Callback for Retro achievements unlock
     * @param ID (Int) : the id of the achievement
     * @param TITLE (String) : the title of the achievement
     * @param DESCRIPTION (String) : the description of the achievements
     * @param POINTS (Int) : the points earnt by this achievement
     * @param BADGE_URL (String) : the url of the badge to be downloaded
     * @param IS_HARDCORE (Boolean) : whether the achievement was unlocked in hardcore mode (no save states)
     * @param TIMESTAMP (Long) : when the achievement was unlcoked
     * @param IS_SILENT (Boolean) : whether to trigger a notification
     */
    @JvmStatic
    fun onAchievementUnlockedCallback(
      ID          : Int,
      TITLE       : String,
      DESCRIPTION : String,
      POINTS      : Int,
      BADGE_URL   : String,
      IS_HARDCORE : Boolean,
      TIMESTAMP   : Long,
      IS_SILENT   : Boolean)
    {
      App.appModule.gameBoy.onAchievementUnlocked(ID, TITLE, DESCRIPTION, POINTS, BADGE_URL, IS_HARDCORE, TIMESTAMP, IS_SILENT)
    }

    /** Deletes all the achievements for current game to load from the server */
    @JvmStatic
    fun clearAchievementsForCurrentGame()
    {
      App.appModule.mainScope.launch()
      {
        val gameUri = staticCurrentRomUri
        if (gameUri != null)
        {
          val game = App.appModule.repo.getGameByUri(gameUri)
          if (game != null)
          {
            ForgeLogger.info("RA: Clearing achievements for game ${game.title} (ID: ${game.id}) to sync from server.")
            App.appModule.database.achievementDAO().deleteAllAchievementsByGameId(game.id)
          }
        }
      }
    }

    /**
     * Retro achievements login success handler
     * @param USERNAME (String) : the username of the person
     * @param TOKEN (String) : the login token
     */
    @JvmStatic
    fun onRaLoginSuccess(USERNAME: String, TOKEN: String)
    {
      App.appModule.mainScope.launch()
      {
        val current = App.appModule.dataStoreManager.getSettings()
        App.appModule.dataStoreManager.updateSettings(current.copy(raUsername = USERNAME, raToken = TOKEN))
      }
    }

    /**
     * Notifies the user of a login error
     * @param MESSAGE (String) : the error message
     */
    @JvmStatic
    fun onRaLoginError(MESSAGE: String)
    {
      App.appModule.mainScope.launch()
      {
        SnackbarController.sendEvent(SnackbarEvent("Login Error: $MESSAGE"))
      }
    }


    // - - - Kotlin Native Audio API - - -

    /** Starts the audio system */
    @JvmStatic
    fun nativeInitAudio()
    { speaker.start() }

    /**
     * Plays a sound created by the emulator
     * @param SAMPLES (FloatArray) : the sound samples created by the emulator
     */
    @JvmStatic
    fun nativePlayAudio(SAMPLES: FloatArray)
    { speaker.play(SAMPLES)    }

    /** Stops the game audio */
    @JvmStatic
    fun nativeStopAudio()
    { speaker.release() }


    // - - - Save file callbacks - - -

    /** Returns the uri of the save file of the game */
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
        val romPathInTree      = romUri.toString().substringAfterLast("/document/")
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

    /**
     * Finds the save file uri
     * @param ROM_PARENT_DIRECTORY (DocumentFile) : The directory of the rom
     * @param ROM_DOCUMENT_FILE (DocumentFile) : the file
     * @return (Uri?) : if found
     */
    private suspend fun resolveSaveFileInParent(
      ROM_PARENT_DIRECTORY: DocumentFile,
      ROM_DOCUMENT_FILE   : DocumentFile): Uri?
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
      val saveFileName  = romFileName.replace(Regex("\\.gbc?$", RegexOption.IGNORE_CASE), ".sav")

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

    /**
     * Makes a save file
     * @param RAM_DATA (ByteArray) : the array of bytes of the ram
     * @param RAM_SIZE (Int) : the size of the ram data array
     * @return (Boolean) : whether the save was successfull
     */
    @JvmStatic
    fun saveRamToFile(RAM_DATA: ByteArray, RAM_SIZE: Int): Boolean
    {
      if (RAM_DATA.size != RAM_SIZE)
      {
        ForgeLogger.error("Kotlin: saveRamToFile: Mismatch in ramData.size (${RAM_DATA.size}) and ramSize ($RAM_SIZE)")
        return false
      }

      val gb          = App.appModule.gameBoy
      val isDeferred  = runBlocking { App.appModule.dataStoreManager.getSettings().isDeferredSavingEnabled }

      if (isDeferred)
      {
        ForgeLogger.info("Kotlin: Deferring RAM save (size: $RAM_SIZE)")
        gb.deferredRamData = RAM_DATA.copyOf()
        gb.isRamDirty      = true
        return true
      }

      ForgeLogger.info("Kotlin: Immediate RAM save (size: $RAM_SIZE)")
      return runBlocking(Dispatchers.IO)
      {
        val saveFileUri = getGameSaveFileUri()
        if (saveFileUri != null)
        {
          val success = App.appModule.externalStorageManager.saveFileFromUri(saveFileUri, RAM_DATA)
          if (success)
          {
            ForgeLogger.info("Kotlin: Successfully saved RAM to $saveFileUri.")
            SnackbarController.sendEvent(SnackbarEvent("Successfully saved game"))
            true
          }
          else
          {
            ForgeLogger.error("Kotlin: Failed to save RAM to $saveFileUri.")
            SnackbarController.sendEvent(SnackbarEvent("Failed to save game : write error"))
            false
          }
        }
        else
        {
          ForgeLogger.error("Kotlin: Could not resolve save file URI. Save failed.")
          SnackbarController.sendEvent(SnackbarEvent("Failed to save game : couldn't resolve save file"))
          false
        }
      }
    }

    /**
     * Executes a final commit of any pending RAM mutations onto background storage.
     */
    @JvmStatic
    fun flushRam()
    {
      val gb = App.appModule.gameBoy
      if (!gb.isRamDirty || gb.deferredRamData == null) return

      val data = gb.deferredRamData!!
      gb.isRamDirty = false
      gb.deferredRamData = null

      ForgeLogger.info("Kotlin: Flushing deferred RAM save (size: ${data.size})")

      runBlocking(Dispatchers.IO)
      {
        val saveFileUri = getGameSaveFileUri()
        if (saveFileUri != null)
        {
          val success = App.appModule.externalStorageManager.saveFileFromUri(saveFileUri, data)
          if (success)
          {
            ForgeLogger.info("Kotlin: Successfully flushed RAM to $saveFileUri.")
            SnackbarController.sendEvent(SnackbarEvent("Game progression saved"))
          }
          else
          {
            ForgeLogger.error("Kotlin: Failed to flush RAM to $saveFileUri.")
            SnackbarController.sendEvent(SnackbarEvent("Failed to save game : write error"))
          }
        }
        else
        {
          ForgeLogger.error("Kotlin: Could not resolve save file URI during flush. Save failed.")
        }
      }
    }

    /**
     * Loads a ram from save file
     * @param RAM_DATA_BUFFER (ByteArray) : the array of bytes for the rame data
     * @param BUFFER_SIZE (Int) : the size of the buffer
     * @return (Boolean) : whether the ram is loaded successfully
     */
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
      App.appModule.gameBoy.pauseEmulator(PauseTrigger.IO)

      val finalResult = runBlocking(Dispatchers.IO)
      {
        val saveFileUri = getGameSaveFileUri()
        val result  =
          if (saveFileUri != null)
          {
            val loadedData = App.appModule.externalStorageManager.readFileFromUri(saveFileUri)
            if (loadedData != null)
            {
              if (loadedData.size == BUFFER_SIZE)
              {
                loadedData.copyInto(RAM_DATA_BUFFER)
                ForgeLogger.info("Kotlin: Successfully loaded ${loadedData.size} bytes from $saveFileUri.")
                SnackbarController.sendEvent(SnackbarEvent("Loaded Game Save"))
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
          App.appModule.gameBoy.resumeEmulator(PauseTrigger.IO)
          result
        }

      return finalResult
    }

    @Deprecated("Save size is now managed by the C++ cartridge core.")
    @JvmStatic
    fun getExpectedSaveSize(): Int
    {
      ForgeLogger.warn("Kotlin: getExpectedSaveSize called. Returning 0 as size is managed by C++ cartridge core.")
      return 0
    }
  }
}