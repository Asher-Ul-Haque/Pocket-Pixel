package just.somebody.templates.presentation.effects

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.repeatOnLifecycle
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.withContext

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
