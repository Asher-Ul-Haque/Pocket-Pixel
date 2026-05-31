package just.somebody.templates.appModule.storage

import android.content.Context
import android.graphics.Bitmap
import just.somebody.templates.App
import just.somebody.templates.data.daos.SaveStateDao
import just.somebody.templates.data.entities.SaveStateEntity
import just.somebody.templates.domain.models.Game
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream

class SaveStateManager(
    private val context: Context,
    private val dao: SaveStateDao
) {
    fun getSaveStatesForGame(gameId: Long): Flow<List<SaveStateEntity>> {
        return dao.getSaveStatesForGame(gameId)
    }

    suspend fun saveState(gameId: Long, slot: Int, data: ByteArray, screenshot: IntArray?) {
        withContext(Dispatchers.IO) {
            val saveState = SaveStateEntity(
                gameId = gameId,
                slot = slot,
                data = data,
                timestamp = System.currentTimeMillis()
            )
            dao.insertSaveState(saveState)
            
            if (screenshot != null) {
                saveScreenshot(gameId, slot, screenshot)
            }
        }
    }

    suspend fun loadState(gameId: Long, slot: Int): ByteArray? {
        return withContext(Dispatchers.IO) {
            dao.getSaveState(gameId, slot)?.data
        }
    }

    private fun saveScreenshot(gameId: Long, slot: Int, pixels: IntArray) {
        val bitmap = Bitmap.createBitmap(pixels, 160, 144, Bitmap.Config.ARGB_8888)
        val file = getScreenshotFile(gameId, slot)
        FileOutputStream(file).use { out ->
            bitmap.compress(Bitmap.CompressFormat.JPEG, 90, out)
        }
    }

    fun getScreenshotFile(gameId: Long, slot: Int): File {
        val dir = File(context.filesDir, "screenshots/$gameId")
        if (!dir.exists()) dir.mkdirs()
        return File(dir, "slot_$slot.jpg")
    }
}
