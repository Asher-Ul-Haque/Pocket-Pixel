package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
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
  private val STORAGE_MANAGER : ExternalStorageManager,
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
      val key = "GAME_BOY_ROMS"
      if (STORAGE_MANAGER.getDirectory(key) == null) return@launch
      val repo = App.appModule.repo
      repo.insertGames(key)
    }
  }

  fun factoryReset()
  {
    viewModelScope.launch ()
    {
      DATASTORE.updateSettings(AppSettings())
      REPO.factoryReset()
      App.appModule.boxArtFetcher.deleteCache()
    }
  }
}