package just.somebody.templates.presentation.screens

import androidx.navigation.NavOptionsBuilder

sealed interface NavigationAction
{
  data class Navigate(
    val DESTINATION : Destination,
    val OPTIONS     : NavOptionsBuilder.() -> Unit = {}
  ) : NavigationAction

  data class PopBackStack(
    val DESTINATION : Destination? = null,
    val INCLUSIVE   : Boolean      = false
  ) : NavigationAction

  data class PopUpTo(
    val DESTINATION : Destination,
    val INCLUSIVE   : Boolean = false
  ) : NavigationAction

  data class  ClearBackStack    (val DESTINATION : Destination) : NavigationAction
  data class  NavigateSingleTop (val DESTINATION : Destination) : NavigationAction
  data class  Replace           (val DESTINATION : Destination) : NavigationAction
  data object NavigateBack                                      : NavigationAction
}