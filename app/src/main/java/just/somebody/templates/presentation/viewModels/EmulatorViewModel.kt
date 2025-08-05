package just.somebody.templates.presentation.viewModels

import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.toPersistentList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class EmulatorViewModel : ViewModel()
{
  private val gameBoy         : GameBoy     = App.appModule.gameBoy
  private var currentROM      : ByteArray?  = null
  private var romReady        : Boolean     = false
  private var emulatorStarted : Boolean     = false
  private val _settings       : MutableStateFlow<AppSettings> = MutableStateFlow<AppSettings>(AppSettings())
  public  val settings        : MutableStateFlow<AppSettings> = _settings

  init
  {
    viewModelScope.launch { _settings.value = App.appModule.dataStoreManager.getSettings() }
  }

  fun stopEmulator()
  {
    viewModelScope.launch()
    {
      gameBoy.stopEmulator()
      romReady        = false
      emulatorStarted = false
    }
  }

  fun runEmulator(URI : String)
  {
    viewModelScope.launch(Dispatchers.IO)
    {
      gameBoy.stopEmulator()

      val romBytes = App.appModule.context
        .contentResolver
        .openInputStream(Uri.parse(URI))
        ?.use { it.readBytes() }

      if (romBytes != null)
      {
        currentROM = romBytes
        romReady = true
        tryStartEmulator(URI)
      }
      else
      {
        SnackbarController.sendEvent(SnackbarEvent("Failed to read ROM data"))
        App.appModule.navigator.replace(Destination.Home)
      }
    }
  }

  private fun tryStartEmulator(URI : String)
  {
    if (!emulatorStarted && romReady && currentROM != null)
    {
      gameBoy.setPalette(_settings.value.paletteIndex)
      gameBoy.setShader(_settings.value.shaderIndex)
      gameBoy.loadROM(currentROM!!, URI)
      gameBoy.startEmulator(_settings.value.channelVolume.toFloatArray())
      emulatorStarted = true
    }
  }

  fun setVolume(VOLUME : Float, INDEX : Int)
  {
    viewModelScope.launch()
    {
      val currentSettings = App.appModule.dataStoreManager.getSettings()

      val newVolumes = currentSettings.channelVolume.toMutableList().apply()
      { this[INDEX % 5] = Math.max(0f, Math.min(1f, VOLUME)) }

      val updatedSettings = currentSettings.copy(channelVolume = newVolumes)

      App.appModule.dataStoreManager.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setVolumes(_settings.value.channelVolume.toFloatArray())
    }
  }

  fun setPaletteIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val currentSettings = App.appModule.dataStoreManager.getSettings()
      val updatedSettings = currentSettings.copy(paletteIndex = INDEX % 8)

      App.appModule.dataStoreManager.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setPalette(INDEX % 8)
    }
  }

  fun setShaderIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val currentSettings = App.appModule.dataStoreManager.getSettings()
      val updatedSettings = currentSettings.copy(shaderIndex = INDEX % 5)

      App.appModule.dataStoreManager.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setShader(INDEX % 5)
    }
  }
}
