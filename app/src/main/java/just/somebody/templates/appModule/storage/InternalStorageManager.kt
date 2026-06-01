package just.somebody.templates.appModule.storage

import android.content.Context
import java.io.File

/**
 * Controller hub managing private application storage boundaries, covering both persistent
 * internal files and non-persistent cache file lifecycles.
 */
interface InternalStorageManager
{
  // - - - persistent files
  /** Validates existence flags for a specific structural filename located inside internal spaces. */
  fun doesFileExist (NAME : String)                           : Boolean

  /** Writes a raw continuous binary block onto disk inside the private `filesDir` sandboxed path. */
  fun saveFile      (NAME : String, FILE_CONTENT : ByteArray) : Boolean

  /** Extracts and returns the full raw byte block mapped by an internal file signature identity. */
  fun readFile      (NAME : String)                           : ByteArray?

  /** Unlinks and deletes a targeted file node instance inside private storage boundaries. */
  fun deleteFile    (NAME : String)                           : Boolean

  /** Compiles a complete list indexing all allocated private file signatures. */
  fun listFiles     ()                                        : List<String>

  /** Creates or opens a specialized directory namespace component inside internal storage workspace roots. */
  fun makeDir       (NAME : String)                           : Boolean

  // - - - cache
  /** Validates explicit file existence states inside temporary system cache spaces. */
  fun doesCacheExist(NAME : String)                           : Boolean

  /** Writes dynamic transient payload data records onto disk inside the application `cacheDir` path. */
  fun cacheFile     (NAME : String, FILE_CONTENT: ByteArray)  : Boolean

  /** Reads out binary payloads hosted inside the fast-discard temporary cache storage pipeline. */
  fun readCache     (NAME : String)                           : ByteArray?

  /** Drops an explicit single record entry from the system cache directory block. */
  fun deleteCache   (NAME : String)                           : Boolean

  /** Wipes out all volatile transient cache contents residing inside the temporary workspace sector. */
  fun clearAllCache ()                                        : Boolean
}

class DefaultInternalStorageManager(private val CONTEXT: Context) : InternalStorageManager
{
  // - - - File Storage (internal, persistent) - - -

  override fun doesFileExist(NAME: String): Boolean
  { return File(CONTEXT.filesDir, NAME).exists() }

  override fun saveFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  {
    return runCatching ()
    {
      CONTEXT.openFileOutput(NAME, Context.MODE_PRIVATE).use { it.write(FILE_CONTENT) }
      true
    }.getOrElse { false }
  }

  override fun readFile(NAME: String): ByteArray?
  {
    return runCatching { CONTEXT.openFileInput(NAME).use { it.readBytes() } }
      .getOrNull()
  }

  override fun deleteFile(NAME: String): Boolean
  { return File(CONTEXT.filesDir, NAME).delete() }

  override fun listFiles(): List<String>
  { return CONTEXT.fileList()?.toList() ?: emptyList() }

  override fun makeDir(NAME: String): Boolean
  {
    val dir = CONTEXT.getDir(NAME, Context.MODE_PRIVATE)
    return dir.exists() && dir.isDirectory
  }

  // - - - Cache Storage (internal, temporary) - - -

  override fun doesCacheExist(NAME: String): Boolean
  { return File(CONTEXT.cacheDir, NAME).exists() }

  override fun cacheFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  {
    return runCatching ()
    {
      val file = File(CONTEXT.cacheDir, NAME)
      file.outputStream().use { it.write(FILE_CONTENT) }
      true
    }.getOrElse { false }
  }

  override fun readCache(NAME: String): ByteArray?
  {
    return runCatching()
    {
      val file = File(CONTEXT.cacheDir, NAME)
      file.inputStream().use { it.readBytes() }
    }.getOrNull()
  }

  override fun deleteCache(NAME: String): Boolean
  { return File(CONTEXT.cacheDir, NAME).delete() }

  override fun clearAllCache(): Boolean
  {
    return runCatching ()
    {
      CONTEXT.cacheDir.listFiles()?.forEach { it.delete() }
      true
    }.getOrElse { false }
  }
}