package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

/**
 * Lifecycle coordinator managing timing sequences during application startup phases.
 *
 * Implements a fixed delay interval to handle asset loading or brand presentations before
 * flipping readiness indicators observed by root platform navigation routers.
 */
class SplashViewModel : ViewModel()
{
  private val _isReady : MutableStateFlow<Boolean> = MutableStateFlow(false)
  public  val isReady  : StateFlow<Boolean>        = _isReady

  init
  {
    viewModelScope.launch ()
    {
      delay(1000)
      _isReady.value = true
    }
  }
}