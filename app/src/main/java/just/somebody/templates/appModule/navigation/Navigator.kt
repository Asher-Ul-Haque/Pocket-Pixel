package just.somebody.templates.appModule.navigation

import androidx.navigation.NavOptionsBuilder
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.receiveAsFlow

/**
 * Interface defining an asynchronous navigation coordinator.
 *
 * Exposes a stream of navigation events that a UI host (like a Composable NavHost) can observe.
 * This decouples the business logic inside ViewModels from context-bound Android components.
 */
interface Navigator
{
  /** The fallback or initial route where the navigation pipeline begins. */
  val startDestination : Destination

  /** A hot flow of navigation requests emitted by features and consumed by the UI engine. */
  val navigationAction : Flow<NavigationAction>

  /**
   * Pushes a new [DESTINATION] onto the controller's active route pipeline.
   *
   * @param DESTINATION The targeted view context to manifest.
   * @param OPTIONS Optional block for fine-grained configuration of anims or target states.
   */
  suspend fun navigate(
    DESTINATION : Destination,
    OPTIONS     : NavOptionsBuilder.() -> Unit = {})

  /** Instructs the state pipeline to revert to the immediate previous screen. */
  suspend fun navigateBack()

  /** Flushes the internal record cache up to a specific [DESTINATION], anchoring it as root. */
  suspend fun clearBackStack    (DESTINATION : Destination)

  /** Routes forward while enforcing an inclusive single instance limit on the target screen. */
  suspend fun navigateSingleTop (DESTINATION : Destination)

  /** Drops the current focus point and swaps it instantly for the targeted [DESTINATION]. */
  suspend fun replace           (DESTINATION : Destination)

  /**
   * Reverts historical navigation points based on explicit parameters.
   *
   * @param DESTINATION Target anchor point to reverse to. If null, steps back once.
   * @param INCLUSIVE Determines if the target parameter anchor is also dropped.
   */
  suspend fun popBackStack(
    DESTINATION : Destination?,
    INCLUSIVE   : Boolean)

  /**
   * Cleans elements in the history pipeline up to a designated terminal target.
   *
   * @param DESTINATION Target bounding node to parse back to.
   * @param INCLUSIVE Setting true also consumes the targeted boundary node.
   */
  suspend fun popUpTo(
    DESTINATION : Destination,
    INCLUSIVE   : Boolean)
}

/**
 * Production implementation of [Navigator] backing actions using Kotlin Coroutine Channels.
 *
 * @property startDestination The fallback or initial screen destination route.
 */
class DefaultNavigator(override val startDestination: Destination) : Navigator
{
  private  val _navigationActions = Channel<NavigationAction>()
  override val navigationAction   = _navigationActions.receiveAsFlow()

  override suspend fun navigate(DESTINATION: Destination, OPTIONS: NavOptionsBuilder.() -> Unit)
  {
    _navigationActions.send(
      NavigationAction.Navigate(
        DESTINATION = DESTINATION,
        OPTIONS = OPTIONS
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