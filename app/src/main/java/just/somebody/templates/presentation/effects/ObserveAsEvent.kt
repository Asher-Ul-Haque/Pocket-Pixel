package just.somebody.templates.presentation.effects

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.repeatOnLifecycle
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext

/**
 * Architectural lifecycle-aware wrapper designed to collect single-fire UI events safely within a Composable node.
 *
 * Automatically manages stream interception properties, pausing collection when the host interface drops below
 * the [Lifecycle.State.STARTED] threshold boundary and resuming collection upon structural reconstruction events.
 * This completely prevents memory leaks and background processing exceptions when ViewModels dispatch non-persistent
 * navigation or notification directives.
 *
 * @param T Structural archetype container tracking data properties dispatched by the stream layer.
 * @param FLOW Target cold data pipeline stream broadcasting message payloads.
 * @param KEY System configuration change tag used to force reconstruction behaviors if needed.
 * @param ON_EVENT Main interception lambda routing incoming payloads directly onto the presentation logic layer.
 */
@Composable
fun <T> ObserveAsEvents(
  FLOW      : Flow<T>,
  KEY       : Any? = null,
  ON_EVENT  : (T) -> Unit
                       )
{
  val lifecycleOwner = LocalLifecycleOwner.current
  LaunchedEffect(lifecycleOwner.lifecycle, KEY, FLOW)
  {
    lifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED)
    {
      withContext(Dispatchers.Main.immediate)
      { FLOW.collect(ON_EVENT) }
    }
  }
}