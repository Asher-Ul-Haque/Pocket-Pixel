package just.somebody.templates.presentation.effects

import androidx.compose.material3.SnackbarDuration
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.receiveAsFlow

/**
 * Packaging configuration mapping custom interactivity capabilities targeting active snackbar notifications.
 *
 * @property name Alphanumeric text segment mapping the execution label displayed inside the actionable anchor.
 * @property action Asynchronous processing function block fired when the user selects the descriptive prompt element.
 */
data class SnackbarAction(
  val name   : String,
  val action : suspend () -> Unit
                         )

/**
 * Data blueprint tracking informational message specifications dispatched into the global notification pipeline.
 *
 * @property message The core descriptive informational text segment presented directly onto the screen overlay.
 * @property action Optional interactive button parameters enabling execution callbacks inside the notification node.
 * @property duration Lifespan configuration mapping how long the target notice remains active on screen fields.
 */
data class SnackbarEvent(
  val message  : String,
  val action   : SnackbarAction?  = null,
  val duration : SnackbarDuration = SnackbarDuration.Short
                        )

/**
 * Unified messaging controller hub coordinating overlay notification dispatches across asynchronous execution pools.
 * Bridges business logic signals directly to the visible presentation layer without forcing close structural dependencies.
 */
object SnackbarController
{
  private val _events = Channel<SnackbarEvent>()
  public  val events  = _events.receiveAsFlow()

  /**
   * Commits a notification task payload down into the operational streaming pipeline.
   *
   * @param EVENT Configured data blueprint tracking message parameters to display.
   */
  suspend fun sendEvent(EVENT : SnackbarEvent)
  { _events.send(EVENT); }
}