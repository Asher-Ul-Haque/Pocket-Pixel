package just.somebody.templates.appModule.storage

import android.content.Context
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

/**
 * Static utility used to pre-allocate required system directories in the ROMs root folder.
 * Ensures 'saves', 'screenshots', 'achievements', and 'boxarts' exist exactly once.
 */
object StorageInitializer {

    /**
     * Sequentially creates all required app folders if they do not exist.
     * This is called on app launch to prevent race conditions during gameplay or downloads.
     */
    suspend fun initialize(context: Context) = withContext(Dispatchers.IO) {
        val storage = App.appModule.externalStorageManager
        val root = storage.getDirectory("GAME_BOY_ROMS") ?: return@withContext
        
        val folders = listOf("saves", "screenshots", "achievements", "boxarts")
        
        ForgeLogger.info("StorageInitializer: Verifying root directory structure...")
        
        for (name in folders) {
            val existing = root.findFile(name)
            if (existing == null || !existing.isDirectory) {
                ForgeLogger.info("StorageInitializer: Creating folder -> $name")
                root.createDirectory(name)
            } else {
                ForgeLogger.trace("StorageInitializer: Folder already exists -> $name")
            }
        }
        
        ForgeLogger.info("StorageInitializer: Root structure verified.")
    }
}
