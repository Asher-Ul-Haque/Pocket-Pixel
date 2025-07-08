package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.presentation.effects.SnackbarAction
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.screens.Destination
import just.somebody.templates.presentation.screens.Navigator
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import kotlin.random.Random

class ScreenAViewModel(private val NAVIGATOR : Navigator) : ViewModel()
{
  val isConnected = App.appModule.hardwareManager
    .isConnectedToInternet
    .stateIn(
      viewModelScope,
      SharingStarted.WhileSubscribed(5000L),
      NetworkStatus.Unavailable
    )

  fun showSnackbar()
  {
    viewModelScope.launch ()
    {
      SnackbarController.sendEvent(
        EVENT = SnackbarEvent(
          message = "Hello from Screen A's viewmodel",
          action  = SnackbarAction(
            name  = "click me",
            action =
              {
                SnackbarController.sendEvent(
                  EVENT = SnackbarEvent("Another Snackbar triggered")
                )
              }
          )
        )
      )
    }
  }

  fun goAway()
  {
    viewModelScope.launch { NAVIGATOR.navigate(Destination.ArgScreen(Random.nextInt())) }
  }
}