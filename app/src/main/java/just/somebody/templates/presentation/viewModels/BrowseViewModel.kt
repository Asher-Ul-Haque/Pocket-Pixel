package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class BrowseViewModel : ViewModel()
{
  private val _selectedIndex : MutableStateFlow<Int> = MutableStateFlow(0)
  public  val selectedIndex  : StateFlow<Int>        = _selectedIndex

  fun onNavigate(NEW_INDEX : Int)
  {
    viewModelScope.launch ()
    {
      if (_selectedIndex.value == NEW_INDEX) return@launch

      _selectedIndex.value = NEW_INDEX
      val destination =
        when (NEW_INDEX)
        {
          1    -> Destination.Favorites
          2    -> Destination.Search
          3    -> Destination.Server
          else -> Destination.Home
        }
      App.appModule.navigator.replace(destination)
    }
  }

  fun goToSettings()
  {
    viewModelScope.launch { App.appModule.navigator.replace(Destination.Settings) }
  }
}