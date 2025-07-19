package just.somebody.templates.domain

import android.graphics.SurfaceTexture
import android.net.Uri
import android.view.Surface
import androidx.core.graphics.createBitmap
import androidx.core.graphics.set
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.ui.theme.GameBoyColors

enum class Buttons {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  A,
  B,
  SELECT,
  START
}

class GameBoy {

  // - - - Memory
  private val audioBuffer = ByteArray(1024) // TODO: Replace with real audio logic

  // - - - Lifecycle Control
  fun prepareSurface(surface: Surface?) {
    nativeSetSurface(surface)
  }

  fun loadROM(ROM: ByteArray) {
    nativeLoadROM(ROM, ROM.size)
  }

  fun startEmulator() {
    nativeStartEmulator()
  }

  fun stopEmulator() {
    nativeStopEmulator()
  }

  fun resetEmulator() {
    nativeStopEmulator()
    nativeStartEmulator()
  }

  // - - - Audio
  fun getAudioBuffer(): ByteArray {
    nativeGetAudioBuffer(audioBuffer)
    return audioBuffer
  }

  // - - - Input
  fun sendButton(
    BUTTON: Buttons,
    IS_PRESSED: Boolean
  ) {
    nativeSetButtonState(BUTTON.ordinal, IS_PRESSED)
  }

  // - - - Native Bindings

  private external fun nativeGetAudioBuffer(AUDIO_BUFFER: ByteArray)
  private external fun nativeLoadROM(ROM: ByteArray, SIZE: Int)
  private external fun nativeSetButtonState(BUTTON: Int, PRESSED: Boolean)
  private external fun nativeStartEmulator()
  private external fun nativeStopEmulator()
  private external fun nativeSetSurface(SURFACE: Surface?)

  companion object {
    init {
      System.loadLibrary("native-lib")
    }
  }
}
