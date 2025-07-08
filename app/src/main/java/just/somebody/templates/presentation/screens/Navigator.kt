package just.somebody.templates.presentation.screens

import androidx.navigation.NavOptionsBuilder
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.receiveAsFlow

interface Navigator
{
  val startDestination : Destination
  val navigationAction : Flow<NavigationAction>

  suspend fun navigate(
    DESTINATION : Destination,
    OPTIONS     : NavOptionsBuilder.() -> Unit = {})

  suspend fun navigateBack()
  suspend fun clearBackStack    (DESTINATION : Destination)
  suspend fun navigateSingleTop (DESTINATION : Destination)
  suspend fun replace           (DESTINATION : Destination)
  suspend fun popBackStack(
    DESTINATION : Destination?,
    INCLUSIVE   : Boolean)
  suspend fun popUpTo(
    DESTINATION : Destination,
    INCLUSIVE   : Boolean)
}

class DefaultNavigator(override val startDestination: Destination) : Navigator
{
  private  val _navigationActions = Channel<NavigationAction>()
  override val navigationAction   = _navigationActions.receiveAsFlow()

  override suspend fun navigate(DESTINATION: Destination, OPTIONS: NavOptionsBuilder.() -> Unit)
  {
    _navigationActions.send(
      NavigationAction.Navigate(
        DESTINATION = DESTINATION,
        OPTIONS     = OPTIONS
      )
    )
  }

  override suspend fun navigateBack()
  { _navigationActions.send(NavigationAction.NavigateBack) }

  override suspend fun popBackStack(DESTINATION : Destination?, INCLUSIVE : Boolean)
  { _navigationActions.send(NavigationAction.PopBackStack(DESTINATION, INCLUSIVE))}

  override suspend fun popUpTo(
    DESTINATION : Destination,
    INCLUSIVE   : Boolean)
  { _navigationActions.send(NavigationAction.PopUpTo(DESTINATION, INCLUSIVE)) }

  override suspend fun clearBackStack(DESTINATION : Destination)
  { _navigationActions.send(NavigationAction.ClearBackStack(DESTINATION)) }

  override suspend fun navigateSingleTop(DESTINATION : Destination)
  { _navigationActions.send(NavigationAction.NavigateSingleTop(DESTINATION))}

  override suspend fun replace(DESTINATION : Destination)
  { _navigationActions.send(NavigationAction.Replace(DESTINATION))}
}