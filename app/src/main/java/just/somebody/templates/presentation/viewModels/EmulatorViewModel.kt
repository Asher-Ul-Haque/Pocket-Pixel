package just.somebody.templates.presentation.viewModels

import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.domain.PauseTrigger
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.models.Palette
import just.somebody.templates.domain.models.PRESET_PALETTES
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.collections.immutable.PersistentList
import kotlinx.collections.immutable.toPersistentList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class EmulatorViewModel : ViewModel()
{
  private val gameBoy         : GameBoy     = App.appModule.gameBoy
  private var currentROM      : ByteArray?  = null
  private var romReady        : Boolean     = false
  private var emulatorStarted : Boolean     = false
  private val _settings       : MutableStateFlow<AppSettings> = MutableStateFlow<AppSettings>(AppSettings())
  public  val settings        : MutableStateFlow<AppSettings> = _settings

  private val _currentGame : MutableStateFlow<Game?> = MutableStateFlow(null)
  val currentGame : StateFlow<Game?> = _currentGame

  private val _fastForward = MutableStateFlow(false)
  val fastForward: StateFlow<Boolean> = _fastForward

  init
  {
    GameBoy.onFirstActivity = {
      // Re-apply settings on first sign of life from the core
      applyCurrentSettings()
      gameBoy.setFastForward(_fastForward.value)
    }
    
    viewModelScope.launch {
      App.appModule.dataStoreManager.settingsFlow.collect { newSettings ->
        _settings.value = newSettings
        // Apply changes immediately if emulator is running
        if (emulatorStarted) {
          applyCurrentSettings(newSettings)
        }
      }
    }
  }

  private fun applyCurrentSettings(S : AppSettings? = null) {
    val current = S ?: _settings.value
    val palette = if (current.paletteIndex < PRESET_PALETTES.size) {
        PRESET_PALETTES[current.paletteIndex]
    } else {
        current.customPalettes.getOrNull(current.paletteIndex - PRESET_PALETTES.size) ?: PRESET_PALETTES[0]
    }
    gameBoy.setPalette(palette)
    gameBoy.setShader(current.shaderIndex)
    gameBoy.setVolumes(current.channelVolume.toFloatArray())
  }

  fun stopEmulator()
  {
    viewModelScope.launch()
    {
      GameBoy.resetActivityFlag()
      gameBoy.stopEmulator()
      romReady        = false
      emulatorStarted = false
      _currentGame.value = null
    }
  }

  fun pause(trigger: PauseTrigger) = gameBoy.pauseEmulator(trigger)
  fun resume(trigger: PauseTrigger) = gameBoy.resumeEmulator(trigger)

  fun runEmulator(URI : String)
  {
    viewModelScope.launch(Dispatchers.IO)
    {
      gameBoy.stopEmulator()

      val game = App.appModule.repo.getGameByUri(URI)
      _currentGame.value = game

      val romBytes = App.appModule.context
        .contentResolver
        .openInputStream(Uri.parse(URI))
        ?.use { it.readBytes() }

      if (romBytes != null)
      {
        currentROM = romBytes
        romReady = true
        incrementLaunchCount()
        tryStartEmulator(URI)
      }
      else
      {
        SnackbarController.sendEvent(SnackbarEvent("Failed to read ROM data"))
        App.appModule.navigator.replace(Destination.Home)
      }
    }
  }

  fun saveState(slot: Int) {
      val gameId = _currentGame.value?.id ?: return
      val data = gameBoy.saveState() ?: return
      val screenshot = gameBoy.nativeCaptureFrame()
      viewModelScope.launch {
          App.appModule.saveStateManager.saveState(gameId, slot, data, screenshot)
          SnackbarController.sendEvent(SnackbarEvent("State saved to Slot $slot"))
      }
  }

  fun loadState(slot: Int) {
      val gameId = _currentGame.value?.id ?: return
      viewModelScope.launch {
          val data = App.appModule.saveStateManager.loadState(gameId, slot)
          if (data != null) {
              if (gameBoy.loadState(data)) {
                  SnackbarController.sendEvent(SnackbarEvent("State loaded from Slot $slot"))
              } else {
                  SnackbarController.sendEvent(SnackbarEvent("Failed to load state"))
              }
          } else {
              SnackbarController.sendEvent(SnackbarEvent("No state in Slot $slot"))
          }
      }
  }

  fun toggleFavorite() {
      val game = _currentGame.value ?: return
      viewModelScope.launch {
          val updatedGame = game.copy(isFavorite = !game.isFavorite)
          App.appModule.repo.updateGame(updatedGame)
          _currentGame.value = updatedGame
      }
  }

  private suspend fun tryStartEmulator(URI : String)
  {
    if (!emulatorStarted && romReady && currentROM != null)
    {
      val currentSettings = App.appModule.dataStoreManager.getSettings()
      _settings.value = currentSettings

      // Order is CRITICAL: loadROM initializes the platform, which resets settings to defaults.
      // We MUST apply our settings AFTER loading the ROM and starting the emulator.
      gameBoy.loadROM(currentROM!!, URI)
      gameBoy.startEmulator()
      
      // Apply settings immediately after launch
      applyCurrentSettings(currentSettings)
      
      emulatorStarted = true
    }
  }

  fun setVolume(VOLUME : Float, INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore = App.appModule.dataStoreManager
      val currentSettings = dataStore.getSettings()

      val newVolumes = currentSettings.channelVolume.toMutableList().apply()
      { this[INDEX % 5] = Math.max(0f, Math.min(1f, VOLUME)) }

      val updatedSettings = currentSettings.copy(channelVolume = newVolumes)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setVolumes(_settings.value.channelVolume.toFloatArray())
    }
  }

  fun setPaletteIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore = App.appModule.dataStoreManager
      val currentSettings = dataStore.getSettings()
      val updatedSettings = currentSettings.copy(paletteIndex = INDEX)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      val palette = if (INDEX < PRESET_PALETTES.size) {
          PRESET_PALETTES[INDEX]
      } else {
          currentSettings.customPalettes.getOrNull(INDEX - PRESET_PALETTES.size) ?: PRESET_PALETTES[0]
      }
      App.appModule.gameBoy.setPalette(palette)
    }
  }

  fun setShaderIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore = App.appModule.dataStoreManager
      val currentSettings = dataStore.getSettings()
      val updatedSettings = currentSettings.copy(shaderIndex = INDEX % 4)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setShader(INDEX % 4)
    }
  }

  private fun incrementLaunchCount() {
      viewModelScope.launch {
          val current = App.appModule.dataStoreManager.getSettings()
          App.appModule.dataStoreManager.updateSettings(current.copy(launchCount = current.launchCount + 1))
      }
  }

  fun toggleFastForward() {
      val newState = !_fastForward.value
      _fastForward.value = newState
      gameBoy.setFastForward(newState)
  }

  fun toggleImmersiveMode() {
      viewModelScope.launch {
          val dataStore = App.appModule.dataStoreManager
          val current = dataStore.getSettings()
          val updated = current.copy(isImmersiveModeEnabled = !current.isImmersiveModeEnabled)
          dataStore.updateSettings(updated)
          _settings.value = updated
      }
  }
}
