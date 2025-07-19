package just.somebody.templates.appModule.storage

import android.content.Context
import java.io.File

interface InternalStorageManager {
  fun doesFileExist(NAME: String): Boolean
  fun saveFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  fun readFile(NAME: String): ByteArray?
  fun deleteFile(NAME: String): Boolean
  fun listFiles(): List<String>
  fun makeDir(NAME: String): Boolean

  fun doesCacheExist(NAME: String): Boolean
  fun cacheFile(NAME: String, FILE_CONTENT: ByteArray): Boolean
  fun readCache(NAME: String): ByteArray?
  fun deleteCache(NAME: String): Boolean
  fun clearAllCache(): Boolean // <-- NEW
}

class DefaultInternalStorageManager(private val CONTEXT: Context) : InternalStorageManager {

  // ----------- File Storage (internal, persistent) -----------

  override fun doesFileExist(NAME: String): Boolean {
    return File(CONTEXT.filesDir, NAME).exists()
  }

  override fun saveFile(NAME: String, FILE_CONTENT: ByteArray): Boolean {
    return runCatching {
      CONTEXT.openFileOutput(NAME, Context.MODE_PRIVATE).use {
        it.write(FILE_CONTENT)
      }
      true
    }.getOrElse {
      false
    }
  }

  override fun readFile(NAME: String): ByteArray? {
    return runCatching {
      CONTEXT.openFileInput(NAME).use { it.readBytes() }
    }.getOrNull()
  }

  override fun deleteFile(NAME: String): Boolean {
    return File(CONTEXT.filesDir, NAME).delete()
  }

  override fun listFiles(): List<String> {
    return CONTEXT.fileList()?.toList() ?: emptyList()
  }

  override fun makeDir(NAME: String): Boolean {
    val dir = CONTEXT.getDir(NAME, Context.MODE_PRIVATE)
    return dir.exists() && dir.isDirectory
  }

  // ----------- Cache Storage (internal, temporary) -----------

  override fun doesCacheExist(NAME: String): Boolean {
    return File(CONTEXT.cacheDir, NAME).exists()
  }

  override fun cacheFile(NAME: String, FILE_CONTENT: ByteArray): Boolean {
    return runCatching {
      val file = File(CONTEXT.cacheDir, NAME)
      file.outputStream().use { it.write(FILE_CONTENT) }
      true
    }.getOrElse {
      false
    }
  }

  override fun readCache(NAME: String): ByteArray? {
    return runCatching {
      val file = File(CONTEXT.cacheDir, NAME)
      file.inputStream().use { it.readBytes() }
    }.getOrNull()
  }

  override fun deleteCache(NAME: String): Boolean {
    return File(CONTEXT.cacheDir, NAME).delete()
  }

  override fun clearAllCache(): Boolean {
    return runCatching {
      CONTEXT.cacheDir.listFiles()?.forEach { it.delete() }
      true
    }.getOrElse {
      false
    }
  }
}
