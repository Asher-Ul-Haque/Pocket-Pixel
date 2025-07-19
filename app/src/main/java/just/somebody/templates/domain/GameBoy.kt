package just.somebody.templates.domain

import android.graphics.Bitmap
import android.net.Uri
import android.view.Surface
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import androidx.core.graphics.createBitmap
import androidx.core.graphics.set

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
  private val audioBuffer = ByteArray(1024)           // - - - TODO : find out how audio on gameboy works
  private var romData     = ByteArray(1024 * 1024 * 4)    // - - - 1MB ROM size
  private val ramData     = ByteArray(1024 * 8)       // - - - 8KB RAM

  // - - - emulation
  fun loadROM(ROM : ByteArray)
  {
    romData = ROM;
    nativeLoadROM(romData, romData.size);
  }

  fun startEmulator  ()                  { nativeStartEmulator(); }
  fun stopEmulator   ()                  {  nativeStopEmulator(); }
  fun resetEmulator  ()
  {
    nativeStopEmulator();
    nativeStartEmulator();
  }

  // - - - video and audio
  fun setSurface(SURFACE : Surface?)
  { nativeSetSurface(SURFACE) }

  fun getAudioBuffer () : ByteArray
  {
    nativeGetAudioBuffer(audioBuffer)
    return audioBuffer
  }

  // - - - input
  fun sendButton(
    BUTTON      : Buttons,
    IS_PRESSED  : Boolean)
  { nativeSetButtonState(BUTTON.ordinal, IS_PRESSED); }


  // - - - Native - - -

  private external fun nativeGetAudioBuffer (AUDIO_BUFFER : ByteArray)
  private external fun nativeLoadROM        (ROM : ByteArray, SIZE : Int)
  private external fun nativeSetButtonState (BUTTON : Int, PRESSED : Boolean)
  private external fun nativeStartEmulator  ();
  private external fun nativeStopEmulator   ();
  private external fun nativeSetSurface     (SURFACE : Surface?)

  // - - - actual c++
  companion object { init { System.loadLibrary("native-lib") } }
}