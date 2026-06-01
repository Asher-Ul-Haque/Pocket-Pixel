package just.somebody.templates.appModule.storage

import android.graphics.Bitmap
import android.content.Context
import just.somebody.templates.data.daos.SaveStateDao
import just.somebody.templates.data.entities.SaveStateEntity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream

/**
 * Business logic controller managing the serialization, persistence, and lookup mappings
 * for active emulation machine state frames and companion preview images.
 *
 * Simultaneously writes historical register blocks directly to the relational database wrapper
 * while converting graphical frame buffers down to physical JPEG imagery assets.
 *
 * @property CONTEXT The platform context reference used to resolve asset file path boundaries.
 * @property DAO Core access mapping layer routing database operations for persistent records.
 */
class SaveStateManager(
  private val CONTEXT: Context,
  private val DAO    : SaveStateDao)
{
  /**
   * Cold stream tracking all historic state entries registered under a specific primary key constraint.
   *
   * @param GAME_ID Database tracking key corresponding to the parent game record.
   * @return Flow containing sequential snapshot listings matching the parameter criteria.
  */
  fun getSaveStatesForGame(GAME_ID: Long): Flow<List<SaveStateEntity>>
  { return DAO.getSaveStatesForGame(GAME_ID) }

  /**
   * Commits a complete running simulation state snapshot and metadata record transaction onto background disk space.
   *
   * @param GAME_ID Database tracking key corresponding to the parent game record.
   * @param SLOT Unique target sector allocation index mapped to this historical slice.
   * @param DATA Raw byte map structure mirroring the current state of the emulator.
   * @param SCREENSHOT Matrix tracking raw pixel structures capturing the visible screen layer.
  */
  suspend fun saveState(GAME_ID: Long, SLOT: Int, DATA: ByteArray, SCREENSHOT: IntArray?)
  {
    withContext(Dispatchers.IO)
    {
      val saveState = SaveStateEntity(
        gameId    = GAME_ID,
        slot      = SLOT,
        data      = DATA,
        timestamp = System.currentTimeMillis())
      DAO.insertSaveState(saveState)

      if (SCREENSHOT != null) saveScreenshot(GAME_ID, SLOT, SCREENSHOT)
    }
  }

  /**
   * Retrieves an isolated state frame mapping block extracted asynchronously out of local database storage.
   *
   * @param GAME_ID Database tracking key corresponding to the parent game record.
   * @param SLOT Unique target sector allocation index mapped to this historical slice.
   * @return Raw state byte map, or null if target entry reference does not exist on disk.
   */
  suspend fun loadState(GAME_ID: Long, SLOT: Int): ByteArray?
  {
    return withContext(Dispatchers.IO)
    { DAO.getSaveState(GAME_ID, SLOT)?.data }
  }

  /**
   * Processes a flat integer pixel array buffer directly down into a compressed JPEG image file layout.
   *
   * Maps across the incoming pixel array block to shift and rearrange mismatched color channels
   *
   * @param GAME_ID Database tracking key corresponding to the parent game record.
   * @param SLOT Unique target sector allocation index mapped to this historical slice.
   * @param PIXELS Raw display coordinate array sizing standard handheld aspect bounds (160x144).
   */
  private fun saveScreenshot(GAME_ID: Long, SLOT: Int, PIXELS: IntArray)
  {
    val correctedPixels = IntArray(PIXELS.size)

    for (i in PIXELS.indices)
    {
      val color = PIXELS[i]
      val alpha = (color ushr 24) and 0xFF
      val red   = color and 0xFF
      val green = (color ushr 8) and 0xFF
      val blue  = (color ushr 16) and 0xFF

      correctedPixels[i] = (alpha shl 24) or (blue shl 16) or (green shl 8) or red
    }

    val bitmap = Bitmap.createBitmap(correctedPixels, 160, 144, Bitmap.Config.ARGB_8888)
    val file   = getScreenshotFile(GAME_ID, SLOT)
    FileOutputStream(file).use()
    { out -> bitmap.compress(Bitmap.CompressFormat.JPEG, 90, out) }
  }

  /**
   * Locates and constructs a verified folder location route matching explicit file parameters on disk.
   *
   * @param GAME_ID Database tracking key corresponding to the parent game record.
   * @param SLOT Unique target sector allocation index mapped to this historical slice.
   * @return A secure pointer location addressing the file node context within system structures.
   */
  fun getScreenshotFile(GAME_ID: Long, SLOT: Int): File
  {
    val dir = File(CONTEXT.filesDir, "screenshots/$GAME_ID")
    if (!dir.exists()) dir.mkdirs()
    return File(dir, "slot_$SLOT.jpg")
  }
}