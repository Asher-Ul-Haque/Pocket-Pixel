package just.somebody.templates.appModule.storage

import android.content.Context
import android.content.Intent
import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.*
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

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

  @Composable
  fun FilePickerLauncher(
    MIME_TYPES : Array<String>,
    ON_PICKED  : (Uri?) -> Unit
  ): () -> Unit

  @Composable
  fun DirectoryPickerLauncher(
    KEY       : String,
    ON_PICKED : (Uri?) -> Unit
  ): () -> Unit

  suspend fun getDirectory(KEY : String) : DocumentFile?
  suspend fun listFiles(
    KEY       : String,
    EXTENSION : String? = null) : List<DocumentFile>
  suspend fun listFilesFromUri(
    URI       : Uri,
    EXTENSION : String? = null) : List<DocumentFile>
  suspend fun saveFile(
    KEY       : String,
    FILE_NAME : String,
    CONTENT   : ByteArray)      : Boolean
  suspend fun readFile(
    KEY       : String,
    FILE_NAME : String)         : ByteArray?
  suspend fun deleteFile(
    KEY       : String,
    FILE_NAME : String)         : Boolean
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

  override suspend fun listFiles(KEY : String, EXTENSION : String?) : List<DocumentFile>
  {
    val dir = getDirectory(KEY)
    if (dir == null)
    {
      ForgeLogger.warn("Cannot list files: directory is null for key = $KEY")
      return emptyList()
    }

    val files = dir.listFiles()
    ForgeLogger.info("Found ${files.size} files in $KEY directory")

    val filtered = files.filter()
    {
      val name = it.name.orEmpty()
      EXTENSION == null || name.endsWith(".$EXTENSION", ignoreCase = true)
    }

    ForgeLogger.info("Filtered ${filtered.size} files with extension '.$EXTENSION'")
    return filtered
  }

  override suspend fun listFilesFromUri(URI : Uri, EXTENSION : String?): List<DocumentFile>
  {
    val dir = DocumentFile.fromTreeUri(CONTEXT, URI)
    if (dir == null || !dir.exists() || !dir.isDirectory)
    {
      ForgeLogger.error("Invalid URI passed to listFilesFromUri: $URI")
      return emptyList()
    }

    val files = dir.listFiles()
    ForgeLogger.info("Listing ${files.size} files from URI: $URI")

    val filtered = files.filter ()
    {
      val name = it.name.orEmpty()
      EXTENSION == null || name.endsWith(".$EXTENSION", ignoreCase = true)
    }

    ForgeLogger.info("Filtered ${filtered.size} matching '.$EXTENSION'")
    return filtered
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
}
