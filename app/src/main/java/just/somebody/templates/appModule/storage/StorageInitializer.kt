package just.somebody.templates.appModule.storage

import android.content.Context
import androidx.annotation.Keep
import androidx.documentfile.provider.DocumentFile
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.concurrent.atomic.AtomicBoolean

/**
 * Static utility used to pre-allocate required system directories in the ROMs root folder.
 * Ensures 'saves', 'screenshots', 'achievements', and 'boxarts' exist exactly once.
 */
@Keep
object StorageInitializer {

    private val isInitialized = AtomicBoolean(false)
    private val isRaAutoLoginTriggered = AtomicBoolean(false)

    /**
     * Sequentially creates all required app folders if they do not exist.
     * This is called on app launch to prevent race conditions during gameplay or downloads.
     */
    suspend fun initialize(context: Context) = withContext(Dispatchers.IO) {
        val storage = App.appModule.externalStorageManager
        val root = storage.getDirectory("GAME_BOY_ROMS") 
        
        if (root == null) {
            ForgeLogger.warn("StorageInitializer: ROMs directory not accessible yet.")
            return@withContext
        }

        if (!isInitialized.get()) {
            val folders = listOf("saves", "screenshots", ".achievements", ".boxarts")
            
            ForgeLogger.info("StorageInitializer: Verifying root directory structure...")
            
            // - - - Get current children to avoid findFile sync issues
            val existingFiles = root.listFiles()
            
            for (name in folders) {
                val exists = existingFiles.any { it.name == name && it.isDirectory }
                if (!exists) {
                    ForgeLogger.info("StorageInitializer: Creating folder -> $name")
                    root.createDirectory(name)
                } else {
                    ForgeLogger.trace("StorageInitializer: Folder verified -> $name")
                }
            }
            
            isInitialized.set(true)
            ForgeLogger.info("StorageInitializer: Root structure verified and locked.")
        }

        // - - - Trigger RetroAchievements Auto-Login if possible
        if (!isRaAutoLoginTriggered.get()) {
            val settings = App.appModule.dataStoreManager.getSettings()
            if (settings.raUsername.isNotEmpty() && settings.raToken.isNotEmpty()) {
                ForgeLogger.info("StorageInitializer: Triggering RetroAchievements auto-login for ${settings.raUsername}")
                isRaAutoLoginTriggered.set(true)
                App.appModule.isRaSyncing.value = true
                App.appModule.gameBoy.raLoginWithToken(settings.raUsername, settings.raToken)
            }
        }
    }
}
