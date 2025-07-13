package just.somebody.templates.domain

import android.graphics.Bitmap
import android.net.Uri
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.domain.models.Game
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

enum class Buttons
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  A,
  B,
  SELECT,
  START
}
class GameBoy
{
  // - - - memory
  private val frameBuffer = ByteArray(160 * 144 / 4)  // - - - 2 bits per pixel
  private val bitmap      = Bitmap.createBitmap(160, 144, Bitmap.Config.ARGB_8888)
  private val audioBuffer = ByteArray(1024)           // - - - TODO : find out how audio on gameboy works
  private var romData     = ByteArray(1024 * 1024)    // - - - 1MB ROM size
  private val ramData     = ByteArray(1024 * 8)       // - - - 8KB RAM

  // - - - emulation
  fun loadROM(URI : String)
  {
    CoroutineScope(Dispatchers.IO).launch ()
    {
      val romBytes = App.appModule.context.contentResolver
        .openInputStream(Uri.parse(URI))
        ?.use { it.readBytes() }

      if (romBytes != null) {
        romData = romBytes
      } else {
        ForgeLogger.error("Failed to read ROM from URI: $URI")
      }
      nativeLoadROM(romData)
    }
  }

  fun startEmulator  ()                  { nativeStartEmulator(); }
  fun stopEmulator   ()                  {  nativeStopEmulator(); }
  fun stepFrame      ()                  {   nativeStepFrame(); }
  fun resetEmulator  ()
  {
    nativeStopEmulator();
    nativeStartEmulator();
  }

  // - - - video and audio
  fun getFrameBuffer () : Bitmap
  {
    nativeGetFrameBuffer(frameBuffer)
    updateBitmapFromBuffer()
    return bitmap
  }
  fun getAudioBuffer () : ByteArray
  {
    nativeGetAudioBuffer(audioBuffer)
    return audioBuffer
  }
  private fun updateBitmapFromBuffer()
  {
    var pixelIndex = 0
    for (byte in frameBuffer)
    {
      for (shift in 6 downTo 0 step 2)
      {
        val colorIndex = (byte.toInt() shr shift) and 0b11
        val x = pixelIndex % 160
        val y = pixelIndex / 160

        bitmap.setPixel(x, y, mapColorToArgb(colorIndex));
        pixelIndex++;
      }
    }
  }
  private fun mapColorToArgb(COLOR_INDEX : Int) : Int
  {
    return when (COLOR_INDEX)
    {
      0    -> GameBoyColors.DarkGreen.toArgb()
      1    -> GameBoyColors.MediumGreen.toArgb()
      2    -> GameBoyColors.Green.toArgb()
      3    -> GameBoyColors.LightGreen.toArgb()
      else -> Color.Black.toArgb()
    }
  }

  // - - - input
  fun sendButton(
    BUTTON      : Buttons,
    IS_PRESSED  : Boolean)
  { nativeSetButtonState(BUTTON.ordinal, IS_PRESSED); }


  // - - - Native - - -

  private external fun nativeStepFrame      ()
  private external fun nativeGetFrameBuffer (FRAME_BUFFER : ByteArray)
  private external fun nativeGetAudioBuffer (AUDIO_BUFFER : ByteArray)
  private external fun nativeLoadROM        (ROM : ByteArray)
  private external fun nativeSetButtonState (BUTTON : Int, PRESSED : Boolean)
  private external fun nativeStartEmulator  ();
  private external fun nativeStopEmulator   ();

  // - - - actual c++
  companion object { init { System.loadLibrary("native-lib") } }
}