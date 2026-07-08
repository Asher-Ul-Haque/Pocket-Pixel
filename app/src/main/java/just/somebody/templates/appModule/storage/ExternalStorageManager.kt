package just.somebody.templates.appModule.storage

import android.net.Uri
import androidx.compose.runtime.Composable
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import android.content.Intent
import android.content.Context
import androidx.compose.runtime.remember
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

/**
 * System bridge managing scoped file systems and persistent storage targets using Android's Storage Access Framework.
 * Resolves directory trees, manages URI permissions across platform sessions, and executes streaming interactions.
 */
interface ExternalStorageManager
{
  companion object
  {
    const val MIME_TEXT   = "text/plain"
    const val MIME_IMAGE  = "image/*"
    const val MIME_AUDIO  = "audio/*"
    const val MIME_VIDEO  = "video/*"
    const val MIME_PDF    = "application/pdf"
    const val MIME_BINARY = "application/octet-stream"
    const val MIME_JSON   = "application/json"
    const val MIME_ZIP    = "application/zip"
    const val MIME_ANY    = "*/*"
  }

  /**
   * Instantiates a runtime single-document workspace file-picking launch hook within a Composable tree context.
   *
   * @param MIME_TYPES Array matrix containing allowed file extensions/types to open.
   * @param ON_PICKED Functional callback invoked when a document resource pointer is obtained.
   * @return Callable invocation lambdas to launch the underlying Activity wrapper.
   */
  @Composable
  fun FilePickerLauncher(
    MIME_TYPES : Array<String>,
    ON_PICKED  : (Uri?) -> Unit
                        ): () -> Unit

  /**
   * Instantiates a runtime tree directory selection framework configuration within a Composable tree context.
   * Automatically requests persistable system permissions and caches the result within [DataStoreManager].
   *
   * @param KEY Internal storage identifier tracking this specified root destination context map.
   * @param ON_PICKED Functional callback invoked when a directory tree reference path is approved.
   * @return Callable invocation lambdas to launch the underlying Activity wrapper.
   */
  @Composable
  fun DirectoryPickerLauncher(
    KEY       : String,
    ON_PICKED : (Uri?) -> Unit
                             ): () -> Unit

  /** Resolves and builds a tracking [DocumentFile] tree directory matched from a known storage [KEY]. */
  suspend fun getDirectory(KEY : String) : DocumentFile?

  /** Scans, collects, and yields [DocumentFile] structural maps found inside a registered storage container. */
  suspend fun listFiles(
    KEY          : String,
    EXTENSION    : String?     = null,
    RECURSIVE    : Boolean     = true,
    IGNORED_DIRS : Set<String> = emptySet()) : List<DocumentFile>

  /** Resolves and returns file structures found within an explicit workspace directory [URI]. */
  suspend fun listFilesFromUri(
    URI          : Uri,
    EXTENSION    : String?     = null,
    RECURSIVE    : Boolean     = true,
    IGNORED_DIRS : Set<String> = emptySet()) : List<DocumentFile>

  /** Materializes a new file asset inside a tracked target directory and stream-writes content into it. */
  suspend fun saveFile(
    KEY       : String,
    FILE_NAME : String,
    CONTENT   : ByteArray)      : Boolean

  /** Resolves a document reference stream under a system keyword layout and pulls down data content. */
  suspend fun readFile(
    KEY       : String,
    FILE_NAME : String)         : ByteArray?

  /** Deletes an explicit file target entry registered within a system location keyword map. */
  suspend fun deleteFile(
    KEY       : String,
    FILE_NAME : String)         : Boolean

  /** Overwrites or writes transactional content directly into an isolated explicit [TARGET_FILE_URI]. */
  suspend fun saveFileFromUri(
    TARGET_FILE_URI : Uri,
    CONTENT         : ByteArray
                             ) : Boolean

  /** Pulls down byte blocks extracted directly from an explicitly targeted file reference location [TARGET_FILE_URI]. */
  suspend fun readFileFromUri(TARGET_FILE_URI : Uri) : ByteArray?

  /** Unlinks or deletes an element entry matching the raw path signature within [TARGET_FILE_URI]. */
  suspend fun deleteFileFromUri(TARGET_FILE_URI : Uri) : Boolean

  /** Resolves or creates a subdirectory within a parent [DocumentFile]. */
  suspend fun getOrCreateDirectory(PARENT: DocumentFile, NAME: String): DocumentFile?
}

class DefaultExternalStorageManager(
  private val CONTEXT             : Context,
  private val DATA_STORE_MANAGER  : DataStoreManager,
                                   ) : ExternalStorageManager
{
  private val contentResolver get() = CONTEXT.contentResolver

  @Composable
  override fun FilePickerLauncher(
    MIME_TYPES : Array<String>,
    ON_PICKED  : (Uri?) -> Unit
                                 ): () -> Unit
  {
    val intent = remember ()
    {
      Intent(Intent.ACTION_OPEN_DOCUMENT).apply ()
      {
        addCategory(Intent.CATEGORY_OPENABLE)
        type =
          if (MIME_TYPES.size == 1) MIME_TYPES[0]
          else                      "*/*"
        putExtra(Intent.EXTRA_MIME_TYPES, MIME_TYPES)
      }
    }

    val launcher = rememberLauncherForActivityResult(ActivityResultContracts.StartActivityForResult())
    { result -> ON_PICKED(result.data?.data) }

    return { launcher.launch(intent) }
  }

  @Composable
  override fun DirectoryPickerLauncher(
    KEY       : String,
    ON_PICKED : (Uri?) -> Unit
                                      ): () -> Unit
  {
    val intent = remember()
    {
      Intent(Intent.ACTION_OPEN_DOCUMENT_TREE).apply()
      {
        addFlags(
          Intent.FLAG_GRANT_READ_URI_PERMISSION or
            Intent.FLAG_GRANT_WRITE_URI_PERMISSION or
            Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION or
            Intent.FLAG_GRANT_PREFIX_URI_PERMISSION
                )
      }
    }

    val launcher = rememberLauncherForActivityResult(ActivityResultContracts.StartActivityForResult())
    { result ->
      val uri = result.data?.data
      if (uri != null)
      {
        try
        {
          contentResolver.takePersistableUriPermission(
            uri,
            Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                                                      )

          CoroutineScope(Dispatchers.IO).launch ()
          {
            val settings        = DATA_STORE_MANAGER.getSettings()
            val updatedSettings = settings.copy(externalUris = settings.externalUris + (KEY to uri.toString()))
            DATA_STORE_MANAGER.updateSettings(updatedSettings)
            ForgeLogger.info("Persisted directory URI for key=$KEY: $uri")
            ON_PICKED(uri)
          }
        }
        catch (e : SecurityException)
        {
          ForgeLogger.error("Failed to persist URI permission: $e")
          ON_PICKED(uri)
        }
      }
      else
      {
        ForgeLogger.warn("Directory picker returned null URI")
        ON_PICKED(null)
      }
    }

    return { launcher.launch(intent) }
  }

  override suspend fun getDirectory(KEY : String) : DocumentFile?
  {
    val uriStr = DATA_STORE_MANAGER.getSettings().externalUris[KEY]
    if (uriStr == null)
    {
      ForgeLogger.warn("No URI found for key: $KEY")
      return null
    }

    val uri     = Uri.parse(uriStr)
    val docFile = DocumentFile.fromTreeUri(CONTEXT, uri)

    if (docFile == null || !docFile.exists() || !docFile.isDirectory)
    {
      ForgeLogger.error("Invalid directory for key=$KEY, uri=$uri")
      return null
    }

    ForgeLogger.info("Resolved directory for key=$KEY: $uri")
    return docFile
  }

  private fun collectFilesRecursively(
    DIRECTORY    : DocumentFile,
    EXTENSION    : String?,
    IGNORED_DIRS : Set<String>
                                     ): List<DocumentFile>
  {
    val result = mutableListOf<DocumentFile>()

    DIRECTORY.listFiles().forEach()
    { file ->
      if (file.isDirectory)
      {
        val dirName = file.name.orEmpty()
        if (dirName !in IGNORED_DIRS)
        {
          result += collectFilesRecursively(file, EXTENSION, IGNORED_DIRS)
        }
      }
      else
      {
        val name = file.name.orEmpty()
        if (EXTENSION == null || name.endsWith(".$EXTENSION", ignoreCase = true))
        { result += file }
      }
    }

    return result
  }

  override suspend fun listFiles(
    KEY          : String,
    EXTENSION    : String?,
    RECURSIVE    : Boolean,
    IGNORED_DIRS : Set<String>
                                ): List<DocumentFile>
  {
    val dir = getDirectory(KEY)
    if (dir == null)
    {
      ForgeLogger.warn("Cannot list files: directory is null for key = $KEY")
      return emptyList()
    }

    val files =
      if (RECURSIVE) collectFilesRecursively(dir, EXTENSION, IGNORED_DIRS)
      else
      {
        dir.listFiles().filter()
        {
          val name = it.name.orEmpty()
          EXTENSION == null || name.endsWith(".$EXTENSION", ignoreCase = true)
        }
      }

    ForgeLogger.info("Found ${files.size} file(s) in $KEY (recursive=$RECURSIVE)")
    return files
  }

  override suspend fun listFilesFromUri(
    URI          : Uri,
    EXTENSION    : String?,
    RECURSIVE    : Boolean,
    IGNORED_DIRS : Set<String>
                                       ): List<DocumentFile>
  {
    val dir = DocumentFile.fromTreeUri(CONTEXT, URI)
    if (dir == null || !dir.exists() || !dir.isDirectory)
    {
      ForgeLogger.error("Invalid URI passed to listFilesFromUri: $URI")
      return emptyList()
    }

    val files =
      if (RECURSIVE) collectFilesRecursively(dir, EXTENSION, IGNORED_DIRS)
      else
      {
        dir.listFiles().filter()
        {
          val name = it.name.orEmpty()
          EXTENSION == null || name.endsWith(".$EXTENSION", ignoreCase = true)
        }
      }

    ForgeLogger.info("Listed ${files.size} file(s) from URI: $URI (recursive=$RECURSIVE)")
    return files
  }

  override suspend fun saveFile(
    KEY       : String,
    FILE_NAME : String,
    CONTENT   : ByteArray): Boolean
  {
    val dir  = getDirectory(KEY) ?: return false
    val file = dir.createFile("application/octet-stream", FILE_NAME) ?: return false

    return try
    {
      contentResolver.openOutputStream(file.uri)?.use { it.write(CONTENT) } ?: return false
      ForgeLogger.info("Saved file $FILE_NAME to key=$KEY")
      true
    }
    catch (e: Exception)
    {
      ForgeLogger.error("Failed to save file $FILE_NAME: $e")
      false
    }
  }

  override suspend fun readFile(
    KEY       : String,
    FILE_NAME : String): ByteArray?
  {
    val dir  = getDirectory(KEY) ?: return null
    val file = dir.listFiles().firstOrNull { it.name == FILE_NAME } ?: return null

    return try
    { contentResolver.openInputStream(file.uri)?.use { it.readBytes() } }
    catch (e: Exception)
    {
      ForgeLogger.error("Failed to read file $FILE_NAME: $e")
      null
    }
  }

  override suspend fun deleteFile(
    KEY       : String,
    FILE_NAME : String) : Boolean
  {
    val dir     = getDirectory(KEY) ?: return false
    val file    = dir.listFiles().firstOrNull { it.name == FILE_NAME } ?: return false
    val deleted = file.delete()
    ForgeLogger.info("Deleted file $FILE_NAME: $deleted")
    return deleted
  }

  override suspend fun saveFileFromUri(
    TARGET_FILE_URI : Uri,
    CONTENT         : ByteArray
                                      ) : Boolean
  {
    val targetFile = DocumentFile.fromSingleUri(CONTEXT, TARGET_FILE_URI)
    if (targetFile == null || !targetFile.canWrite())
    {
      ForgeLogger.error("Failed to resolve or write to target save file URI: $TARGET_FILE_URI")
      return false
    }

    return try
    {
      contentResolver.openOutputStream(targetFile.uri)?.use { it.write(CONTENT) } ?: return false
      ForgeLogger.info("Saved file to URI: $TARGET_FILE_URI")
      true
    }
    catch (e: Exception)
    {
      ForgeLogger.error("Failed to save file to URI $TARGET_FILE_URI: $e")
      false
    }
  }

  override suspend fun readFileFromUri(TARGET_FILE_URI : Uri) : ByteArray?
  {
    val targetFile = DocumentFile.fromSingleUri(CONTEXT, TARGET_FILE_URI)
    if (targetFile == null || !targetFile.exists() || !targetFile.isFile || !targetFile.canRead())
    {
      ForgeLogger.warn("Failed to resolve or read from target save file URI: $TARGET_FILE_URI")
      return null
    }

    return try
    { contentResolver.openInputStream(targetFile.uri)?.use { it.readBytes() } }
    catch (e: Exception)
    {
      ForgeLogger.error("Failed to read file from URI $TARGET_FILE_URI: $e")
      null
    }
  }

  override suspend fun deleteFileFromUri(TARGET_FILE_URI : Uri) : Boolean
  {
    val targetFile = DocumentFile.fromSingleUri(CONTEXT, TARGET_FILE_URI)
    if (targetFile == null || !targetFile.exists() || !targetFile.canWrite())
    {
      ForgeLogger.warn("Failed to resolve or delete target save file URI: $TARGET_FILE_URI")
      return false
    }
    val deleted = targetFile.delete()
    ForgeLogger.info("Deleted file from URI $TARGET_FILE_URI: $deleted")
    return deleted
  }

  override suspend fun getOrCreateDirectory(PARENT: DocumentFile, NAME: String): DocumentFile?
  {
    return PARENT.findFile(NAME) ?: PARENT.createDirectory(NAME)
  }
}
