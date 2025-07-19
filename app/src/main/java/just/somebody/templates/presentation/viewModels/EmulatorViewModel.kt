package just.somebody.templates.presentation.viewModels

import android.net.Uri
import android.view.Surface
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class EmulatorViewModel : ViewModel() {

  private val gameBoy: GameBoy = App.appModule.gameBoy

  private var currentROM: ByteArray? = null
  private var romReady: Boolean = false
  private var emulatorStarted: Boolean = false


  fun stopEmulator() {
    viewModelScope.launch {
      gameBoy.stopEmulator()
      romReady = false
      emulatorStarted = false
    }
  }

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



  private fun tryStartEmulator() {
    if (!emulatorStarted && romReady && currentROM != null) {
      gameBoy.loadROM(currentROM!!)
      gameBoy.startEmulator()
      emulatorStarted = true
    }
  }
}
