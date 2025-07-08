package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.domain.Repository
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

class MainViewModel(
  private val REPO             : Repository,
  private val SETTINGS_MANAGER : DataStoreManager
) : ViewModel()
{
  private val appSettings : StateFlow<AppSettings> = SETTINGS_MANAGER.settingsFlow.stateIn(
    viewModelScope,
    SharingStarted.Lazily,
    AppSettings()
  )

  fun doSomething()
  {
    viewModelScope.launch { REPO.doSomething() }
  }

  fun updateSomething(SOMETHING : Int)
  {
    viewModelScope.launch ()
    {
      val current = appSettings.value
      SETTINGS_MANAGER.updateSettings(current.copy(something = SOMETHING))
    }
  }


}