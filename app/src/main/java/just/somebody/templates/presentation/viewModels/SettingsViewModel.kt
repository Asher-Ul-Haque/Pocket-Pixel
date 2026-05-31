package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.repositories.GameRepository
import just.somebody.templates.presentation.effects.SnackbarAction
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class SettingsViewModel(
  private val REPO            : GameRepository,
  private val DATASTORE       : DataStoreManager
) : ViewModel()
{
  private val _settings : MutableStateFlow<AppSettings> = MutableStateFlow<AppSettings>(AppSettings())
  public  val settings  : StateFlow<AppSettings>        = _settings

  init
  {
    viewModelScope.launch { _settings.value = DATASTORE.getSettings() }
  }

  fun rescan()
  {
    viewModelScope.launch ()
    {
      val key                     = "GAME_BOY_ROMS"
      val repo                    = App.appModule.repo
      repo.syncGamesWithStorage(key)
    }
  }

  fun updateControllerConnection() {
    App.appModule.gameControllerManager.updateConnectionState()
  }

  fun setPalette(index: Int) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          DATASTORE.updateSettings(current.copy(paletteIndex = index))
          _settings.value = DATASTORE.getSettings()
      }
  }

  fun setShader(index: Int) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          DATASTORE.updateSettings(current.copy(shaderIndex = index))
          _settings.value = DATASTORE.getSettings()
      }
  }

  fun setVolume(volume: Float, index: Int) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          val newVolumes = current.channelVolume.toMutableList()
          newVolumes[index] = volume.coerceIn(0f, 1f)
          DATASTORE.updateSettings(current.copy(channelVolume = newVolumes))
          _settings.value = DATASTORE.getSettings()
          App.appModule.gameBoy.setVolumes(newVolumes.toFloatArray())
      }
  }

  fun setGamepadButtonMapping(keyCode: Int, gbButton: Buttons?) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          val newMap = current.gamepadMapping.buttonToGameBoy.toMutableMap()
          if (gbButton == null) newMap.remove(keyCode)
          else newMap[keyCode] = gbButton
          
          val newGamepadMapping = current.gamepadMapping.copy(buttonToGameBoy = newMap)
          DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
          _settings.value = DATASTORE.getSettings()
      }
  }

  fun setGamepadAxisMapping(axis: Int, direction: Int, gbButton: Buttons?) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          val newAxisMap = current.gamepadMapping.axisToGameBoy.toMutableMap()
          val directionMap = newAxisMap[axis]?.toMutableMap() ?: mutableMapOf()
          
          if (gbButton == null) directionMap.remove(direction)
          else directionMap[direction] = gbButton
          
          if (directionMap.isEmpty()) newAxisMap.remove(axis)
          else newAxisMap[axis] = directionMap

          val newGamepadMapping = current.gamepadMapping.copy(axisToGameBoy = newAxisMap)
          DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
          _settings.value = DATASTORE.getSettings()
      }
  }

  fun setDeadzone(value: Float) {
      viewModelScope.launch {
          val current = DATASTORE.getSettings()
          val newGamepadMapping = current.gamepadMapping.copy(deadzone = value)
          DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
          _settings.value = DATASTORE.getSettings()
      }
  }

  fun factoryReset()
  {
    viewModelScope.launch ()
    {
      DATASTORE.updateSettings(AppSettings())
      REPO.factoryReset()
      App.appModule.boxArtFetcher.deleteCache()
      App.appModule.dataStoreManager.clearSettings()
    }
  }
}