package just.somebody.templates.appModule.storage

import android.content.Context
import java.io.File

interface InternalStorageManager
{
  fun doesFileExist (NAME: String) : Boolean
  fun saveFile      (NAME : String, FILE_CONTENT : ByteArray) : Boolean
  fun readFile      (NAME : String)                           : ByteArray?
  fun deleteFile    (NAME : String)                           : Boolean
  fun listFiles     ()                                        : List<String>
  fun makeDir       (NAME : String)                           : Boolean

  fun doesCacheExist(NAME : String)                           : Boolean
  fun cacheFile     (NAME : String, FILE_CONTENT : ByteArray) : Boolean
  fun readCache     (NAME : String)                           : ByteArray?
  fun deleteCache   (NAME : String)                           : Boolean
}

class DefaultInternalStorageManager(private val CONTEXT : Context) : InternalStorageManager
{
  override fun doesFileExist(NAME: String): Boolean
  { return File(CONTEXT.filesDir, NAME).exists() }

  override fun saveFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  {
    try
    {
      CONTEXT.openFileOutput(NAME, Context.MODE_PRIVATE).use { it.write(FILE_CONTENT) }
      return true;
    }
    catch (e : Exception) { return false; }
  }

  override fun readFile(NAME: String): ByteArray?
  {
    try
    { return CONTEXT.openFileInput(NAME).use { it.readBytes() } }
    catch (e : Exception)
    { return null }
  }

  override fun deleteFile(NAME: String): Boolean
  { return CONTEXT.deleteFile(NAME); }

  override fun listFiles(): List<String>
  { return CONTEXT.fileList()?.toList() ?: emptyList() }

  override fun makeDir(NAME : String) : Boolean
  {
    val file : File =  CONTEXT.getDir(NAME, Context.MODE_PRIVATE)
    return file.isDirectory
  }


  override fun doesCacheExist(NAME: String): Boolean
  { return File(CONTEXT.cacheDir, NAME).exists() }

  override fun cacheFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  {
    try
    {
      val cacheFIle = File(CONTEXT.cacheDir, NAME)
      cacheFIle.outputStream().use { it.write(FILE_CONTENT) }
      return true;
    }
    catch (e : Exception)
    { return false }
  }

  override fun readCache(NAME : String): ByteArray?
  {
    try
    {
      val file = File(CONTEXT.cacheDir, NAME)
      return file.inputStream().use { it.readBytes() }
    }
    catch (e : Exception)
    { return null }
  }

  override fun deleteCache(NAME: String): Boolean
  {
    val file = File(CONTEXT.cacheDir, NAME)
    return file.delete()
  }
}