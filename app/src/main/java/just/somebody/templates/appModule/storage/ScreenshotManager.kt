package just.somebody.templates.appModule.storage

import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.Uri
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.ByteArrayOutputStream
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/**
 * Controller hub managing the capturing, persistence, and viewing of game screenshots in external storage.
 */
class ScreenshotManager(private val context: Context)
{
  /**
   * Saves a raw pixel buffer as a PNG screenshot for a specific game in the external ROMs directory.
   */
  suspend fun saveScreenshot(gameTitle: String, pixels: IntArray) = withContext(Dispatchers.IO)
  {
    try
    {
      val correctedPixels = IntArray(pixels.size)
      for (i in pixels.indices)
      {
        val color = pixels[i]
        val alpha = (color ushr 24) and 0xFF
        val red   = color and 0xFF
        val green = (color ushr 8) and 0xFF
        val blue  = (color ushr 16) and 0xFF
        correctedPixels[i] = (alpha shl 24) or (red shl 16) or (green shl 8) or blue
      }

      val bitmap = Bitmap.createBitmap(correctedPixels, 160, 144, Bitmap.Config.ARGB_8888)
      val stream = ByteArrayOutputStream()
      bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream)
      val content = stream.toByteArray()

      val storage = App.appModule.externalStorageManager
      val root    = storage.getDirectory("GAME_BOY_ROMS") ?: return@withContext false
      
      val screenshotsDir = storage.getOrCreateDirectory(root, "screenshots") ?: return@withContext false
      val gameDir        = storage.getOrCreateDirectory(screenshotsDir, gameTitle.filter { it.isLetterOrDigit() || it.isWhitespace() }) ?: return@withContext false

      val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault()).format(Date())
      val fileName  = "${gameTitle.filter { it.isLetterOrDigit() }}_$timestamp.png"
      
      val file = gameDir.createFile("image/png", fileName) ?: return@withContext false
      context.contentResolver.openOutputStream(file.uri)?.use { it.write(content) }

      ForgeLogger.info("Screenshot saved to external storage: ${file.uri}")
      true
    }
    catch (e: Exception)
    {
      ForgeLogger.error("Failed to save screenshot: $e")
      false
    }
  }

  /**
   * Opens the directory containing screenshots for a specific game.
   */
  fun openScreenshotsForGame(gameTitle: String)
  {
    App.appModule.mainScope.launch(Dispatchers.IO)
    {
      val storage = App.appModule.externalStorageManager
      val root    = storage.getDirectory("GAME_BOY_ROMS") ?: return@launch
      val screenshotsDir = storage.getOrCreateDirectory(root, "screenshots") ?: return@launch
      val gameDir        = storage.getOrCreateDirectory(screenshotsDir, gameTitle.filter { it.isLetterOrDigit() || it.isWhitespace() }) ?: return@launch
      
      openDirectory(gameDir.uri)
    }
  }

  /**
   * Opens the root screenshots directory in external storage.
   */
  fun openAllScreenshots()
  {
    App.appModule.mainScope.launch(Dispatchers.IO)
    {
      val storage = App.appModule.externalStorageManager
      val root    = storage.getDirectory("GAME_BOY_ROMS") ?: return@launch
      val screenshotsDir = storage.getOrCreateDirectory(root, "screenshots") ?: return@launch
      
      openDirectory(screenshotsDir.uri)
    }
  }

  private fun openDirectory(uri: Uri)
  {
    try
    {
      val intent = Intent(Intent.ACTION_VIEW)
      intent.setDataAndType(uri, "vnd.android.document/directory")
      intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
      intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      
      context.startActivity(intent)
    }
    catch (e: Exception)
    {
      ForgeLogger.error("Could not open directory via SAF: $e")
      // Fallback: try to just open the URI
      try {
          val fallbackIntent = Intent(Intent.ACTION_VIEW, uri)
          fallbackIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
          fallbackIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
          context.startActivity(fallbackIntent)
      } catch (e2: Exception) {
          ForgeLogger.error("Fallback open directory failed: $e2")
      }
    }
  }
}
