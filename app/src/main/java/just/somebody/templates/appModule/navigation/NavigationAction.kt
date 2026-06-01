package just.somebody.templates.appModule.navigation

import androidx.navigation.NavOptionsBuilder
import just.somebody.templates.presentation.screens.Destination

/**
 * Defines the set of navigation behaviors available throughout the application.
 *
 * This sealed interface acts as a command pattern for the app's navigation architecture,
 * allowing ViewModels or other components to emit intent-based navigation events without
 * coupling directly to the Jetpack Compose navigation controller context.
 *
 * @see Navigator
 * @see Destination
 */
sealed interface NavigationAction
{
  /**
   * Navigates to a specific [DESTINATION] with optional custom [OPTIONS].
   *
   * @property DESTINATION The target screen destination to open.
   * @property OPTIONS A lambda to configure Jetpack Compose [NavOptionsBuilder] (e.g., animations, launching single top).
   */
  data class Navigate(
    val DESTINATION : Destination,
    val OPTIONS     : NavOptionsBuilder.() -> Unit = {}) : NavigationAction

  /**
   * Pops the back stack down to a specific [DESTINATION].
   *
   * @property DESTINATION The destination to pop back to. If null, the immediate previous screen is popped.
   * @property INCLUSIVE If true, the specified [DESTINATION] will also be popped off the stack.
   */
  data class PopBackStack(
    val DESTINATION : Destination? = null,
    val INCLUSIVE   : Boolean      = false) : NavigationAction

  /**
   * Pops screens up until the specified [DESTINATION] is reached.
   *
   * @property DESTINATION The target boundary destination to stop popping at.
   * @property INCLUSIVE If true, pops the [DESTINATION] itself as well.
   */
  data class PopUpTo(
    val DESTINATION : Destination,
    val INCLUSIVE   : Boolean = false) : NavigationAction

  /**
   * Clears the entire back stack and sets the specified [DESTINATION] as the new root.
   *
   * Useful for scenarios like logging out or resetting the app state.
   *
   * @property DESTINATION The destination that will become the new base of the back stack.
   */
  data class ClearBackStack(val DESTINATION : Destination) : NavigationAction

  /**
   * Navigates to a [DESTINATION] while ensuring there is at most one copy of it
   * at the top of the back stack stack.
   *
   * Prevents multiple instances of the same screen when a user taps a button repeatedly.
   *
   * @property DESTINATION The target screen destination.
   */
  data class NavigateSingleTop(val DESTINATION : Destination) : NavigationAction

  /**
   * Replaces the current top screen with the new [DESTINATION] by popping the current
   * screen off before navigating.
   *
   * @property DESTINATION The destination to replace the current screen with.
   */
  data class Replace(val DESTINATION : Destination) : NavigationAction

  /**
   * Navigates exactly one step back in the standard history stack.
   */
  data object NavigateBack : NavigationAction
}