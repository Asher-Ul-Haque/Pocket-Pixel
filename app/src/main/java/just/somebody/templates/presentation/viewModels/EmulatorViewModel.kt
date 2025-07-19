package just.somebody.templates.presentation.viewModels

import android.graphics.Bitmap
import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class EmulatorViewModel : ViewModel()
{
  private val _frameSignal= MutableStateFlow(0)
  val frameSignal: StateFlow<Int> = _frameSignal


  private val gameBoy = App.appModule.gameBoy

  fun runEmulator(uri: String) {
    viewModelScope.launch(Dispatchers.Default) {
      gameBoy.stopEmulator()

      val romBytes = withContext(Dispatchers.IO) {
        App.appModule.context
          .contentResolver
          .openInputStream(Uri.parse(uri))
          ?.use { it.readBytes() }
      }

      if (romBytes != null) {
        gameBoy.loadROM(romBytes)
        gameBoy.startEmulator()
      } else {
        SnackbarController.sendEvent(SnackbarEvent("Failed to read ROM data"))
        App.appModule.navigator.replace(Destination.Home)
        return@launch
      }

      while (true)
      {
        val frameStart = System.nanoTime()


        _frameSignal.emit(_frameSignal.value + 1)


        gameBoy.stepFrame()


        val frameTime = System.nanoTime() - frameStart
        val frameTimeMillis = frameTime / 1_000_000
        val targetFrameMillis = 16L

        val sleepMillis = targetFrameMillis - frameTimeMillis
        if (sleepMillis > 0) {
          delay(sleepMillis)
        }
      }
    }
  }
}
