package just.somebody.templates.presentation.viewModels

import androidx.compose.ui.res.stringResource
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

data class BrowseUIState
(
  val selectedIndex  : Int,
  val showInfoDialog : Boolean,
  val showTopBar     : Boolean,
  val showBottomBar  : Boolean
)

class BrowseViewModel : ViewModel()
{
  private val _browseState : MutableStateFlow<BrowseUIState> = MutableStateFlow(
    BrowseUIState(
      0,
      false,
      true,
      true)
  )
  public  val browseState  : StateFlow<BrowseUIState>        = _browseState

  fun onNavigate(NEW_INDEX : Int)
  {
    viewModelScope.launch ()
    {
      if (_browseState.value.selectedIndex == NEW_INDEX) return@launch

      _browseState.update ()
      {
        BrowseUIState(
        NEW_INDEX,
        _browseState.value.showInfoDialog,
        _browseState.value.showTopBar,
        _browseState.value.showBottomBar)
      }
      val destination =
        when (NEW_INDEX)
        {
          1    -> Destination.Favorites
          2    -> Destination.Search
          3    -> Destination.Server
          4    -> Destination.Settings
          else -> Destination.Home
        }
      App.appModule.navigator.replace(destination)
    }
  }

  fun goToSettings()
  {
    viewModelScope.launch { App.appModule.navigator.navigate(Destination.Settings) }
  }

  fun toggleSeeInfo()
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          !_browseState.value.showInfoDialog,
          _browseState.value.showTopBar,
          _browseState.value.showBottomBar)
      }
    }
  }

  fun getDestinationTitle() : Int
  {
    return when (_browseState.value.selectedIndex)
    {
      1    -> R.string.FAV
      2    -> R.string.SEARCH
      3    -> R.string.SERVER
      else -> R.string.HOME
    }
  }

  fun toggleTopBar(TOGGLE : Boolean)
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          _browseState.value.showInfoDialog,
          TOGGLE,
          _browseState.value.showBottomBar)
      }
    }
  }

  fun toggleBottomBar(TOGGLE: Boolean)
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          _browseState.value.showInfoDialog,
          _browseState.value.showTopBar,
          TOGGLE)
      }
    }
  }

  fun toggleBars(BOTTOM : Boolean, TOP : Boolean)
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          _browseState.value.showInfoDialog,
          TOP,
          BOTTOM)
      }
    }
  }
}