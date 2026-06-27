package just.somebody.templates.appModule.storage

import android.content.Context
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.network.NetworkResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

/**
 * Manager handling persistent asset caching (boxarts, achievement badges) within the user's ROMs directory.
 * Ensures offline availability and handles cache invalidation when remote links change.
 *
 * @param CACHE_DIR_NAME (String) : the cache subdirectory
 * @param BOXARTS_DIR_NAME (String) : the boxart subdirectory
 * @param ACHIEVEMENTS_DIR_NAME (String) : the achievement subdirectory
 */
class LocalAssetManager(private val CONTEXT: Context)
{
  private val CACHE_DIR_NAME        = ".pixel_cache"
  private val BOXARTS_DIR_NAME      = "boxarts"
  private val ACHIEVEMENTS_DIR_NAME = "achievements"

    /**
     * Stores a mapping of file name to its source URL to handle invalidation.
     * @param FILE_NAME (String) : the file name of the image
     * @param URL (String) : the url of the new image
     */
    private suspend fun updateUrlMapping(FILE_NAME: String, URL: String)
    {
      val dataStore = App.appModule.dataStoreManager
      val settings  = dataStore.getSettings()
      val updated   = settings.copy(assetUrlMapping = settings.assetUrlMapping + (FILE_NAME to URL))
      dataStore.updateSettings(updated)
    }

    /**
     * Returns the url from a file name of the image
     * @param FILE_NAME (String) : the name of the file
     * @return (String?) : the name of the url if found
     */
    private suspend fun getUrlMapping(FILE_NAME: String): String?
    {
      return App
        .appModule.dataStoreManager
        .getSettings()
        .assetUrlMapping[FILE_NAME]
    }

    /**
     * Resolves the local cache directory for a specific category.
     * @param CATEGORY (String) : the category for which the directory is to be searched
     * @return (DocumentFile?) : the cache directory if found
     */
    private suspend fun getCacheDirectory(CATEGORY: String): DocumentFile? = withContext(Dispatchers.IO)
    {
      val storage = App.appModule.externalStorageManager
      val romsDir = storage.getDirectory(App.appModule.gameRomsKey) ?: return@withContext null
        
      val pixelCacheDir = storage.getOrCreateDirectory(romsDir, CACHE_DIR_NAME) ?: return@withContext null
      return@withContext storage.getOrCreateDirectory(pixelCacheDir, CATEGORY)
    }

    /**
     * Downloads an asset if it doesn't exist locally or if it needs to be updated.
     * @param URL The remote URL of the asset.
     * @param FILE_NAME The local filename to save as.
     * @param CATEGORY The subfolder category (boxarts or achievements).
     * @return The local Uri of the cached asset, or the original URL if download fails.
     */
    suspend fun getCachedAsset(
      URL       : String,
      FILE_NAME : String,
      CATEGORY  : String): String = withContext(Dispatchers.IO)
    {
      if (URL.isEmpty()) return@withContext ""

      val cacheDir  = getCacheDirectory(CATEGORY) ?: return@withContext URL
      val file      = cacheDir.findFile(FILE_NAME)
      val mappedUrl = getUrlMapping(FILE_NAME)

      // - - - If file exists but URL changed, delete it
      if (file != null && mappedUrl != null && mappedUrl != URL)
      {
        ForgeLogger.info("Asset URL changed, invalidating cache: $FILE_NAME")
        file.delete()
      }
      else if (file != null && file.exists())
      {
        return@withContext file.uri.toString()
      }

      // - - - Download and save
      ForgeLogger.info("Downloading asset to cache: $URL -> $FILE_NAME")
      val result = App.appModule.networkService.get<ByteArray>(URL)
      if (result is NetworkResult.Success)
      {
        val newFile = cacheDir.createFile("image/png", FILE_NAME) ?: return@withContext URL
        CONTEXT
          .contentResolver
          .openOutputStream(newFile.uri)
          ?.use { it.write(result.data) }
        updateUrlMapping(FILE_NAME, URL)
        ForgeLogger.info("Asset cached successfully: ${newFile.uri}")
        return@withContext newFile.uri.toString()
      }
      else
      {
        ForgeLogger.error("Failed to download asset: $URL")
        return@withContext URL
      }
    }

    /**
     * Deletes a cached asset.
     * @param FILE_NAME (String) : the name of the file to be deleted
     * @param CATEGORY (String) : the category of the asset
     */
    suspend fun deleteCachedAsset(FILE_NAME: String, CATEGORY: String) = withContext(Dispatchers.IO)
    {
      val cacheDir = getCacheDirectory(CATEGORY) ?: return@withContext
      cacheDir.findFile(FILE_NAME)?.delete()
    }

    companion object
    {
      const val CATEGORY_BOXARTS        = "boxarts"
      const val CATEGORY_ACHIEVEMENTS   = "achievements"
    }
}
