package just.somebody.templates.appModule.storage

import android.content.Context
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.network.NetworkResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext

/**
 * Manager handling persistent asset caching (boxarts, achievement badges) within the user's ROMs directory.
 * Rely on StorageInitializer to ensure folders exist.
 */
class LocalAssetManager(private val CONTEXT: Context)
{
    private val downloadMutex = Mutex()

    /**
     * Stores a mapping of file name to its source URL to handle invalidation.
     */
    private suspend fun updateUrlMapping(FILE_NAME: String, URL: String)
    {
      val dataStore = App.appModule.dataStoreManager
      val settings  = dataStore.getSettings()
      val updated   = settings.copy(assetUrlMapping = settings.assetUrlMapping + (FILE_NAME to URL))
      dataStore.updateSettings(updated)
    }

    private suspend fun getUrlMapping(FILE_NAME: String): String?
    {
      return App.appModule.dataStoreManager.getSettings().assetUrlMapping[FILE_NAME]
    }

    /**
     * Resolves a top-level directory (achievements, boxarts) without attempting creation.
     */
    private suspend fun getTargetDirectory(CATEGORY: String): DocumentFile? = withContext(Dispatchers.IO) {
        val storage = App.appModule.externalStorageManager
        val root = storage.getDirectory("GAME_BOY_ROMS") ?: return@withContext null
        return@withContext root.findFile(CATEGORY) ?: root.listFiles().find { it.name == CATEGORY && it.isDirectory }
    }

    /**
     * Synchronously checks for a cached asset and returns its local URI if it exists.
     */
    suspend fun getLocalAssetUri(FILE_NAME: String, CATEGORY: String): String? = withContext(Dispatchers.IO) {
        val targetDir = getTargetDirectory(CATEGORY) ?: return@withContext null
        val file = targetDir.findFile(FILE_NAME)
        return@withContext if (file != null && file.exists() && file.isFile) file.uri.toString() else null
    }

    /**
     * Downloads an asset directly into pre-allocated folders.
     * Parallel network fetch, sequential file system write.
     */
    suspend fun downloadToCache(
      URL       : String,
      FILE_NAME : String,
      CATEGORY  : String,
      FORCE     : Boolean = false): String? {
      if (URL.isEmpty()) return null

      // 1. QUICK CHECK: If file already exists, return local URI immediately (no lock)
      if (!FORCE)
      {
        getLocalAssetUri(FILE_NAME, CATEGORY)?.let { return it }
      }

      // 2. NETWORK FETCH (Parallel allowed, no lock)
      val result = App.appModule.networkService.get<ByteArray>(URL)
      if (result !is NetworkResult.Success) return null

      // 3. FILE SYSTEM WRITE (Sequential via mutex)
      return downloadMutex.withLock {
        withContext(Dispatchers.IO)
        {
          val storage = App.appModule.externalStorageManager
          val root = storage.getDirectory("GAME_BOY_ROMS") ?: return@withContext null
          
          // FRESH RESOLUTION: Always find the folder again inside the lock
          val targetDir = root.findFile(CATEGORY) ?: run {
              ForgeLogger.error("LocalAssetManager: Target directory $CATEGORY MISSING!")
              return@withContext null
          }
          
          val existingFile = targetDir.findFile(FILE_NAME)
          val mappedUrl = getUrlMapping(FILE_NAME)

          // - - - Invalidate if URL changed OR FORCE is true
          if (existingFile != null && (FORCE || (mappedUrl != null && mappedUrl != URL)))
          {
            existingFile.delete()
          }
          else if (existingFile != null && existingFile.exists() && existingFile.isFile)
          {
            return@withContext existingFile.uri.toString()
          }

          // - - - Save binary data
          try {
              val newFile = targetDir.createFile("image/png", FILE_NAME) ?: return@withContext null
              CONTEXT.contentResolver.openOutputStream(newFile.uri)?.use { it.write(result.data) }
              updateUrlMapping(FILE_NAME, URL)
              return@withContext newFile.uri.toString()
          } catch (e: Exception) {
              ForgeLogger.error("LocalAssetManager: FAILED to save downloaded file: $e")
              null
          }
        }
      }
    }

    /**
     * Deletes all cached assets.
     */
    suspend fun clearAllCache() = withContext(Dispatchers.IO) {
        getTargetDirectory(CATEGORY_ACHIEVEMENTS)?.listFiles()?.forEach { it.delete() }
        getTargetDirectory(CATEGORY_BOXARTS)?.listFiles()?.forEach { it.delete() }
        
        val dataStore = App.appModule.dataStoreManager
        val settings = dataStore.getSettings()
        dataStore.updateSettings(settings.copy(assetUrlMapping = emptyMap()))
    }

    companion object
    {
      const val CATEGORY_BOXARTS        = ".boxarts"
      const val CATEGORY_ACHIEVEMENTS   = ".achievements"
    }
}
