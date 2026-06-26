package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.appModule.storage.dataStore.GamepadMapping
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.repositories.GameRepository
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

/**
 * Data coordinator managing mutations across global app preferences, hardware controller mappings, and index wipes.
 *
 * Runs background dispatch requests to commit modifications across serialized Proto DataStore profiles, local core audio registers,
 * and persistent data tables safely.
 *
 * @property REPO Domain abstraction layer handling local file data queries.
 * @property DATASTORE Persistent system interface wrapper supervising transactional updates to preference files.
 */
class SettingsViewModel(
  private val REPO            : GameRepository,
  private val DATASTORE       : DataStoreManager) : ViewModel()
{
  private val _settings : MutableStateFlow<AppSettings> = MutableStateFlow<AppSettings>(AppSettings())
  public  val settings  : StateFlow<AppSettings>        = _settings

  init
  {
    viewModelScope.launch { _settings.value = DATASTORE.getSettings() }
  }

  /** Invokes synchronization logic sheets aligning application database structures with external system folders. */
  fun rescan()
  {
    viewModelScope.launch ()
    {
      val key                     = "GAME_BOY_ROMS"
      val repo                    = App.appModule.repo
      repo.syncGamesWithStorage(key)
      SoundController.playSound(SoundEffect.Ping)
    }
  }

  /** Signals input managers to evaluate connection parameters for physical external hardware attachments. */
  fun updateControllerConnection() { 
    App.appModule.gameControllerManager.updateConnectionState()
    viewModelScope.launch { SoundController.playSound(SoundEffect.Ping2) }
  }

  /** Modifies selection pointers tracking target visualization filters inside preference stores. */
  fun setPalette(INDEX: Int)
  {
    viewModelScope.launch()
    {
      val current = DATASTORE.getSettings()
      DATASTORE.updateSettings(current.copy(paletteIndex = INDEX))
      _settings.value = DATASTORE.getSettings()
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Modifies tracking parameters adjusting active graphical filters within system properties files. */
  fun setShader(INDEX: Int)
  {
    viewModelScope.launch()
    {
      val current = DATASTORE.getSettings()
      DATASTORE.updateSettings(current.copy(shaderIndex = INDEX))
      _settings.value = DATASTORE.getSettings()
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Computes clamped level values across specialized channels and propagates configurations down onto audio components. */
  fun setVolume(VOLUME: Float, INDEX: Int)
  {
    viewModelScope.launch()
    {
      val current     : AppSettings         = DATASTORE.getSettings()
      val newVolumes  : MutableList<Float>  = current.channelVolume.toMutableList()
      newVolumes[INDEX] = VOLUME.coerceIn(0f, 1f)
      DATASTORE.updateSettings(current.copy(channelVolume = newVolumes))
      _settings.value = DATASTORE.getSettings()
      App.appModule.gameBoy.setVolumes(newVolumes.toFloatArray())
    }
  }

  /** Maps custom digital hardware click signals to target emulator interface buttons. */
  fun setGamepadButtonMapping(KEY_CODE: Int, GB_BUTTON: Buttons?)
  {
    viewModelScope.launch()
    {
      val current : AppSettings               = DATASTORE.getSettings()
      val newMap  : MutableMap<Int, Buttons>  = current.gamepadMapping.buttonToGameBoy.toMutableMap()
      if (GB_BUTTON == null)  newMap.remove(KEY_CODE)
      else                    newMap[KEY_CODE] = GB_BUTTON

      val newGamepadMapping = current.gamepadMapping.copy(buttonToGameBoy = newMap)
      DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
      _settings.value = DATASTORE.getSettings()
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Maps custom continuous directional joystick motions to specific emulator navigation coordinates. */
  fun setGamepadAxisMapping(AXIS: Int, DIRCETION: Int, GB_BUTTON: Buttons?)
  {
    viewModelScope.launch()
    {
      val current      : AppSettings                        = DATASTORE.getSettings()
      val newAxisMap   : MutableMap<Int, Map<Int, Buttons>> = current.gamepadMapping.axisToGameBoy.toMutableMap()
      val directionMap : MutableMap<Int, Buttons>           = newAxisMap[AXIS]?.toMutableMap() ?: mutableMapOf()

      if (GB_BUTTON == null)  directionMap.remove(DIRCETION)
      else                    directionMap[DIRCETION] = GB_BUTTON

      if (directionMap.isEmpty()) newAxisMap.remove(AXIS)
      else                        newAxisMap[AXIS] = directionMap

      val newGamepadMapping = current.gamepadMapping.copy(axisToGameBoy = newAxisMap)
      DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
      _settings.value = DATASTORE.getSettings()
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Updates deadzone margins to filter input drift fluctuations within analog tracking matrices. */
  fun setDeadzone(VALUE: Float)
  {
    viewModelScope.launch()
    {
      val current           : AppSettings     = DATASTORE.getSettings()
      val newGamepadMapping : GamepadMapping  = current.gamepadMapping.copy(deadzone = VALUE)
      DATASTORE.updateSettings(current.copy(gamepadMapping = newGamepadMapping))
      _settings.value = DATASTORE.getSettings()
    }
  }

  /** Adjusts system configurations to toggle immersive edge margins across display layouts. */
  fun toggleImmersiveMode()
  {
    viewModelScope.launch()
    {
      val current = DATASTORE.getSettings()
      val updated = current.copy(isImmersiveModeEnabled = !current.isImmersiveModeEnabled)
      DATASTORE.updateSettings(updated)
      _settings.value = updated
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Toggles between immediate and deferred RAM saving modes. */
  fun toggleDeferredSaving()
  {
    viewModelScope.launch()
    {
      val current = DATASTORE.getSettings()
      val updated = current.copy(isDeferredSavingEnabled = !current.isDeferredSavingEnabled)
      DATASTORE.updateSettings(updated)
      _settings.value = updated
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Toggles RetroAchievements Hardcore Mode. */
  fun toggleRaHardcoreMode()
  {
    viewModelScope.launch()
    {
      val current = DATASTORE.getSettings()
      val updated = current.copy(isRaHardcoreEnabled = !current.isRaHardcoreEnabled)
      DATASTORE.updateSettings(updated)
      _settings.value = updated
      App.appModule.gameBoy.raSetHardcoreMode(updated.isRaHardcoreEnabled)
      SoundController.playSound(SoundEffect.Ping2)
    }
  }

  /** Drops database tables, resets serialized preferences files, and flushes image lookup indices completely. */
  fun factoryReset()
  {
    viewModelScope.launch ()
    {
      DATASTORE.updateSettings(AppSettings())
      REPO.factoryReset()
      App.appModule.boxArtFetcher.deleteCache()
      App.appModule.dataStoreManager.clearSettings()
      SoundController.playSound(SoundEffect.Ping)
    }
  }
}
