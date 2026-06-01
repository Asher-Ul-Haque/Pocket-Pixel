package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

/**
 * Immutable architecture layout snapshot tracking structural presentation variables for shell components.
 *
 * @property selectedIndex Main location pointer identifying the currently targeted navigation tab coordinate.
 * @property showInfoDialog Toggles systemic application documentation visibility modals.
 * @property showSettings Intercepts visual overrides to manifest overlay property adjustments.
 * @property showBars Toggles contextual operational menu layers (headers and bottom navigation views).
 */
data class BrowseUIState
(
  val selectedIndex  : Int,
  val showInfoDialog : Boolean,
  val showSettings   : Boolean,
  val showBars       : Boolean,
)

/**
 * Presentation layer coordinator hosting control actions for the global navigation interface framework.
 * Pipes active navigation changes through manual injection routers to switch presentation screens smoothly.
 */
class BrowseViewModel : ViewModel()
{
  private val _browseState : MutableStateFlow<BrowseUIState> = MutableStateFlow(
    BrowseUIState(
      0,
      false,
      false,
      true))
  public  val browseState  : StateFlow<BrowseUIState>        = _browseState

  /**
   * Commits an atomic navigation adjustment operation across asynchronous thread pipelines.
   * Swaps context states via the central [Navigator] to refresh structural screen overlays.
   *
   * @param NEW_INDEX Coordinate identifier mapping the target path selection segment.
   */
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
          _browseState.value.showSettings,
          _browseState.value.showBars)
      }
      val destination =
        when (NEW_INDEX)
        {
          1    -> Destination.Favorites
          2    -> Destination.Search
          3    -> Destination.Settings
          else -> Destination.Home
        }
      App.appModule.navigator.replace(destination)
    }
  }

  /** Shifts layout configurations to alternate the presentation layer visibility of internal settings. */
  fun goToSettings(TOGGLE : Boolean)
  {
    viewModelScope.launch ()
    {
      _browseState.update()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          _browseState.value.showInfoDialog,
          TOGGLE,
          _browseState.value.showBars)
      }
    }
  }

  /** Toggles display conditions for documentation elements across active layout subtrees. */
  fun toggleSeeInfo()
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          !_browseState.value.showInfoDialog,
          _browseState.value.showSettings,
          _browseState.value.showBars)
      }
    }
  }

  /** Extracts and returns localized string resource identities aligned to active presentation states. */
  fun getDestinationTitle() : Int
  {
    return when (_browseState.value.selectedIndex)
    {
      1    -> R.string.FAV
      2    -> R.string.SEARCH
      3    -> R.string.SETTINGS
      else -> R.string.HOME
    }
  }

  /** Modifies visibility rules affecting operational shell layers based on runtime configurations. */
  fun showBars(TOGGLE : Boolean)
  {
    viewModelScope.launch ()
    {
      _browseState.update ()
      {
        BrowseUIState(
          _browseState.value.selectedIndex,
          _browseState.value.showInfoDialog,
          _browseState.value.showSettings,
          TOGGLE)
      }
    }
  }
}