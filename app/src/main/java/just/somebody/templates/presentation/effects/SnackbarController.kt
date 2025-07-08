package just.somebody.templates.presentation.effects

import androidx.compose.material3.SnackbarDuration
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.receiveAsFlow

data class SnackbarAction(
  val name   : String,
  val action : suspend () -> Unit
)


data class SnackbarEvent(
  val message  : String,
  val action   : SnackbarAction?  = null,
  val duration : SnackbarDuration = SnackbarDuration.Short
)

object SnackbarController
{
  private val _events = Channel<SnackbarEvent>()
  public  val events  = _events.receiveAsFlow()

  suspend fun sendEvent(EVENT : SnackbarEvent)
  { _events.send(EVENT); }
}