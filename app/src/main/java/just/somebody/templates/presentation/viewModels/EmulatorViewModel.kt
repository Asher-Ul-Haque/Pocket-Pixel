package just.somebody.templates.presentation.viewModels

import android.graphics.Bitmap
import android.net.Uri
import android.view.Surface
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class EmulatorViewModel : ViewModel() {
  private val _frameSignal = MutableStateFlow(0)
  val frameSignal: StateFlow<Int> = _frameSignal

  private val gameBoy = App.appModule.gameBoy
  private var currentROM: ByteArray? = null
  private var surfaceReady = false
  private var romReady = false

  fun runEmulator(uri: String) {
    viewModelScope.launch(Dispatchers.IO) {
      gameBoy.stopEmulator()

      val romBytes = App.appModule.context
        .contentResolver
        .openInputStream(Uri.parse(uri))
        ?.use { it.readBytes() }

      if (romBytes != null) {
        currentROM = romBytes
        romReady = true
        tryStartEmulator()
      } else {
        SnackbarController.sendEvent(SnackbarEvent("Failed to read ROM data"))
        App.appModule.navigator.replace(Destination.Home)
      }
    }
  }

  fun setNativeSurface(SURFACE: Surface?) {
    gameBoy.setSurface(SURFACE)
    surfaceReady = SURFACE != null
    tryStartEmulator()
  }

  private fun tryStartEmulator() {
    if (surfaceReady && romReady && currentROM != null) {
      gameBoy.loadROM(currentROM!!)
      gameBoy.startEmulator()
    }
  }

  override fun onCleared() {
    gameBoy.stopEmulator()
    surfaceReady = false
    romReady = false
  }
}
