package just.somebody.templates.presentation.viewModels

import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.domain.PauseTrigger
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.models.PRESET_PALETTES
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

/**
 * Execution coordinator managing operational data mappings between the user layout and the native emulation core.
 * Runs thread-isolated streaming IO requests to load software assets, manipulate system states, and update user settings.
 */
class EmulatorViewModel : ViewModel()
{
  private val gameBoy         : GameBoy     = App.appModule.gameBoy
  private var currentROM      : ByteArray?  = null
  private var romReady        : Boolean     = false
  private var emulatorStarted : Boolean     = false
  private val _settings       : MutableStateFlow<AppSettings> = MutableStateFlow<AppSettings>(AppSettings())
  public  val settings        : MutableStateFlow<AppSettings> = _settings

  private val _currentGame : MutableStateFlow<Game?> = MutableStateFlow(null)
  val currentGame          : StateFlow<Game?> = _currentGame

  private val _fastForward : MutableStateFlow<Boolean> = MutableStateFlow(false)
  val fastForward          : StateFlow<Boolean> = _fastForward

  init
  {
    GameBoy.onFirstActivity =
      {
        // - - - Re-apply settings on first sign of life from the core
        applyCurrentSettings()
        gameBoy.setFastForward(_fastForward.value)
      }

    viewModelScope.launch()
    {
      App.appModule.dataStoreManager.settingsFlow.collect()
      { newSettings ->
        _settings.value = newSettings
        // - - - Apply changes immediately if emulator is running
        if (emulatorStarted)
        {
          applyCurrentSettings(newSettings)
        }
      }
    }
  }

  /** Maps configured user preference parameters down onto the underlying execution registers of the machine. */
  private fun applyCurrentSettings(SETTINGS : AppSettings? = null)
  {
    val current = SETTINGS ?: _settings.value
    val palette =
      if (current.paletteIndex < PRESET_PALETTES.size)
      { PRESET_PALETTES[current.paletteIndex] }
      else
      { current.customPalettes.getOrNull(current.paletteIndex - PRESET_PALETTES.size) ?: PRESET_PALETTES[0] }
    gameBoy.setPalette(palette)
    gameBoy.setShader(current.shaderIndex)
    gameBoy.setVolumes(current.channelVolume.toFloatArray())
  }

  /** Tears down runtime execution contexts, resets execution state registers, and flushes binary caches. */
  fun stopEmulator()
  {
    viewModelScope.launch()
    {
      GameBoy.resetActivityFlag()
      gameBoy.stopEmulator()
      romReady            = false
      emulatorStarted     = false
      _currentGame.value  = null
    }
  }

  /** Halts internal clock cycle iterations within the core execution loops. */
  fun pause(TRIGGER: PauseTrigger) = gameBoy.pauseEmulator(TRIGGER)

  /** Resumes clock registration ticks within the core execution loops. */
  fun resume(TRIGGER: PauseTrigger) = gameBoy.resumeEmulator(TRIGGER)

  /**
   * Initializes, decodes, and establishes execution scopes targeting an external software asset URI.
   *
   * @param URI Platform Storage Access Framework string target pinpointing the destination item.
   */
  fun runEmulator(URI : String)
  {
    viewModelScope.launch(Dispatchers.IO)
    {
      gameBoy.stopEmulator()

      val game           = App.appModule.repo.getGameByUri(URI)
      _currentGame.value = game

      val romBytes = App.appModule.context
        .contentResolver
        .openInputStream(Uri.parse(URI))
        ?.use { it.readBytes() }

      if (romBytes != null)
      {
        currentROM = romBytes
        romReady   = true
        incrementLaunchCount()
        tryStartEmulator(URI)
      }
      else
      {
        SnackbarController.sendEvent(SnackbarEvent(App.appModule.context.getString(R.string.failed_rom_data)))
        App.appModule.navigator.replace(Destination.Home)
      }
    }
  }

  /** Extracts current execution state snapshots and flushes the binary context directly onto storage files. */
  fun saveState(SLOT: Int)
  {
    val gameId : Long       = _currentGame.value?.id ?: return
    val data   : ByteArray  = gameBoy.saveState() ?: return
    val screenshot = gameBoy.nativeCaptureFrame()
    viewModelScope.launch()
    {
      App.appModule.saveStateManager.saveState(gameId, SLOT, data, screenshot)
      SnackbarController.sendEvent(SnackbarEvent(App.appModule.context.getString(R.string.state_saved, SLOT)))
    }
  }

  /** Pulls down historical memory snapshots from disk storage and forces register injection updates. */
  fun loadState(slot: Int)
  {
    val gameId = _currentGame.value?.id ?: return
    viewModelScope.launch()
    {
      val data = App.appModule.saveStateManager.loadState(gameId, slot)
      if (data != null)
      {
        if (gameBoy.loadState(data))
        { SnackbarController.sendEvent(SnackbarEvent(App.appModule.context.getString(R.string.state_loaded, slot))) }
        else { SnackbarController.sendEvent(SnackbarEvent(App.appModule.context.getString(R.string.failed_load_state))) }
      }
      else
      { SnackbarController.sendEvent(SnackbarEvent(App.appModule.context.getString(R.string.no_state_in_slot, slot))) }
    }
  }

  /** Inverts priority parameters across active structures and commits adjustments to local database tables. */
  fun toggleFavorite()
  {
    val game = _currentGame.value ?: return
    viewModelScope.launch()
    {
      val updatedGame = game.copy(isFavorite = !game.isFavorite)
      App.appModule.repo.updateGame(updatedGame)
      _currentGame.value = updatedGame
    }
  }

  /** Unpacks compiled binary buffers, hooks system routing references, and engages native thread loops. */
  private suspend fun tryStartEmulator(URI : String)
  {
    if (!emulatorStarted && romReady && currentROM != null)
    {
      val currentSettings = App.appModule.dataStoreManager.getSettings()
      _settings.value = currentSettings

      gameBoy.loadROM(currentROM!!, URI)
      gameBoy.startEmulator()
      applyCurrentSettings(currentSettings)

      emulatorStarted = true
    }
  }

  /** Adjusts sound levels across specific hardware layout dimensions and saves settings updates. */
  fun setVolume(VOLUME : Float, INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore       : DataStoreManager    = App.appModule.dataStoreManager
      val currentSettings : AppSettings         = dataStore.getSettings()
      val newVolumes      : MutableList<Float>  = currentSettings.channelVolume.toMutableList().apply()
      { this[INDEX % 4] = Math.max(0f, Math.min(1f, VOLUME)) }

      val updatedSettings = currentSettings.copy(channelVolume = newVolumes)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setVolumes(_settings.value.channelVolume.toFloatArray())
    }
  }

  /** Swaps targeted retro visualization layouts and updates systemic color mapping matrices. */
  fun setPaletteIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore       : DataStoreManager  = App.appModule.dataStoreManager
      val currentSettings : AppSettings       = dataStore.getSettings()
      val updatedSettings : AppSettings       = currentSettings.copy(paletteIndex = INDEX)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      val palette =
        if (INDEX < PRESET_PALETTES.size) { PRESET_PALETTES[INDEX] }
        else
        { currentSettings.customPalettes.getOrNull(INDEX - PRESET_PALETTES.size) ?: PRESET_PALETTES[0] }
      App.appModule.gameBoy.setPalette(palette)
    }
  }

  /** Swaps systemic filtering configurations affecting output canvas matrix blocks. */
  fun setShaderIndex(INDEX : Int)
  {
    viewModelScope.launch()
    {
      val dataStore       : DataStoreManager  = App.appModule.dataStoreManager
      val currentSettings : AppSettings       = dataStore.getSettings()
      val updatedSettings : AppSettings       = currentSettings.copy(shaderIndex = INDEX % 4)

      dataStore.updateSettings(updatedSettings)
      _settings.value = updatedSettings

      App.appModule.gameBoy.setShader(INDEX % 4)
    }
  }

  /** Increments tracking values logging the total software launches executed across cycles. */
  private fun incrementLaunchCount()
  {
    viewModelScope.launch()
    {
      val current = App.appModule.dataStoreManager.getSettings()
      App.appModule.dataStoreManager.updateSettings(current.copy(launchCount = current.launchCount + 1))
    }
  }

  /** Modifies performance execution flags to engage or disengage accelerated processing. */
  fun toggleFastForward()
  {
    val newState        = !_fastForward.value
    _fastForward.value  = newState
    gameBoy.setFastForward(newState)
  }

  /** Switches screen mode flags to maximize visual layouts over display structures. */
  fun toggleImmersiveMode()
  {
    viewModelScope.launch()
    {
      val dataStore : DataStoreManager  = App.appModule.dataStoreManager
      val current   : AppSettings       = dataStore.getSettings()
      val updated   : AppSettings       = current.copy(isImmersiveModeEnabled = !current.isImmersiveModeEnabled)
      dataStore.updateSettings(updated)
      _settings.value = updated
    }
  }
}