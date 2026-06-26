package just.somebody.templates.appModule.storage

import android.content.Context
import android.net.Uri
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.network.NetworkResult
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

/**
 * Manager handling persistent asset caching (boxarts, achievement badges) within the user's ROMs directory.
 * Ensures offline availability and handles cache invalidation when remote links change.
 */
class LocalAssetManager(private val context: Context) {

    private val CACHE_DIR_NAME = ".pixel_cache"
    private val BOXARTS_DIR_NAME = "boxarts"
    private val ACHIEVEMENTS_DIR_NAME = "achievements"

    /**
     * Stores a mapping of file name to its source URL to handle invalidation.
     */
    private suspend fun updateUrlMapping(fileName: String, url: String) {
        val dataStore = App.appModule.dataStoreManager
        val settings = dataStore.getSettings()
        val updated = settings.copy(assetUrlMapping = settings.assetUrlMapping + (fileName to url))
        dataStore.updateSettings(updated)
    }

    private suspend fun getUrlMapping(fileName: String): String? {
        return App.appModule.dataStoreManager.getSettings().assetUrlMapping[fileName]
    }

    /**
     * Resolves the local cache directory for a specific category.
     */
    private suspend fun getCacheDirectory(category: String): DocumentFile? = withContext(Dispatchers.IO) {
        val storage = App.appModule.externalStorageManager
        val romsDir = storage.getDirectory("GAME_BOY_ROMS") ?: return@withContext null
        
        val pixelCacheDir = storage.getOrCreateDirectory(romsDir, CACHE_DIR_NAME) ?: return@withContext null
        return@withContext storage.getOrCreateDirectory(pixelCacheDir, category)
    }

    /**
     * Downloads an asset if it doesn't exist locally or if it needs to be updated.
     * @param url The remote URL of the asset.
     * @param fileName The local filename to save as.
     * @param category The subfolder category (boxarts or achievements).
     * @return The local Uri of the cached asset, or the original URL if download fails.
     */
    suspend fun getCachedAsset(url: String, fileName: String, category: String): String = withContext(Dispatchers.IO) {
        if (url.isEmpty()) return@withContext ""

        val cacheDir = getCacheDirectory(category) ?: return@withContext url
        val file = cacheDir.findFile(fileName)
        
        val mappedUrl = getUrlMapping(fileName)

        // If file exists but URL changed, delete it
        if (file != null && mappedUrl != null && mappedUrl != url) {
            ForgeLogger.info("Asset URL changed, invalidating cache: $fileName")
            file.delete()
        } else if (file != null && file.exists()) {
            return@withContext file.uri.toString()
        }

        // Download and save
        ForgeLogger.info("Downloading asset to cache: $url -> $fileName")
        val result = App.appModule.networkService.get<ByteArray>(url)
        if (result is NetworkResult.Success) {
            val newFile = cacheDir.createFile("image/png", fileName) ?: return@withContext url
            context.contentResolver.openOutputStream(newFile.uri)?.use { it.write(result.data) }
            updateUrlMapping(fileName, url)
            ForgeLogger.info("Asset cached successfully: ${newFile.uri}")
            return@withContext newFile.uri.toString()
        } else {
            ForgeLogger.error("Failed to download asset: $url")
            return@withContext url
        }
    }

    /**
     * Deletes a cached asset.
     */
    suspend fun deleteCachedAsset(fileName: String, category: String) = withContext(Dispatchers.IO) {
        val cacheDir = getCacheDirectory(category) ?: return@withContext
        cacheDir.findFile(fileName)?.delete()
    }

    companion object {
        const val CATEGORY_BOXARTS = "boxarts"
        const val CATEGORY_ACHIEVEMENTS = "achievements"
    }
}
