package just.somebody.templates.presentation.viewModels

import androidx.annotation.Keep
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.StorageInitializer
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

/**
 * Lifecycle coordinator managing timing sequences during application startup phases.
 */
@Keep
class SplashViewModel : ViewModel()
{
  private val _isReady : MutableStateFlow<Boolean> = MutableStateFlow(false)
  public  val isReady  : StateFlow<Boolean>        = _isReady

  init
  {
    viewModelScope.launch ()
    {
      // - - - Ensure system collections exist
      App.appModule.collectionRepo.ensureSystemCollections()
      
      // - - - Perform storage initialization and RA auto-login during splash
      StorageInitializer.initialize(App.appModule.context)

      _isReady.value = true
    }
  }
}
