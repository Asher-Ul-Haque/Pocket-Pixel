package just.somebody.templates.presentation.widgets

import android.annotation.SuppressLint
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.unit.dp
import just.somebody.templates.ui.theme.GameBoyColors

/**
 * A customized, pixel-perfect tactile progress slider styled for a retro look.
 *
 * It uses raw pointer event stream scopes to process immediate taps or sliding drag coordinates
 * across the component's absolute canvas width. This bypasses the typical round-thumb Material layout
 * in favor of a solid filled status block.
 *
 * @param VALUE A normalized percentage float bounded between `0.0f` and `1.0f`.
 * @param ON_VALUE_CHANGE Callback event pipeline delivering calculated position shifts back to the state tracker.
 * @param MODIFIER [Modifier] used to establish outer positioning constraints or sizing layout math.
 * @param ACTIVE_COLOR The primary [Color] representation showing the progress fill bar. Defaults to [GameBoyColors.Green].
 * @param INACTIVE_COLOR The background trough [Color] layer visible beneath unselected spaces. Defaults to [GameBoyColors.DarkGreen].
 * @param BORDER_COLOR Outer hard bounding stroke line [Color]. Defaults to [GameBoyColors.Green].
 */
@SuppressLint("UnusedBoxWithConstraintsScope")
@Composable
fun RetroSlider(
  VALUE           : Float,
  ON_VALUE_CHANGE : (Float) -> Unit,
  MODIFIER        : Modifier = Modifier,
  ACTIVE_COLOR    : Color = GameBoyColors.Green,
  INACTIVE_COLOR  : Color = GameBoyColors.DarkGreen,
  BORDER_COLOR    : Color = GameBoyColors.Green)
{
  BoxWithConstraints(
    modifier = MODIFIER
      .height(24.dp)
      .border(2.dp, BORDER_COLOR, RectangleShape)
      .background(INACTIVE_COLOR)
      .pointerInput(Unit)
      {
        detectTapGestures()
        { offset ->
          ON_VALUE_CHANGE((offset.x / size.width).coerceIn(0f, 1f))
        }
      }
      .pointerInput(Unit)
      {
        awaitPointerEventScope()
        {
          while (true)
          {
            val event = awaitPointerEvent()
            if (event.changes.any { it.pressed })
            {
              val position = event.changes.first().position
              ON_VALUE_CHANGE((position.x / size.width).coerceIn(0f, 1f))
            }
          }
        }
      })
  {
    val sliderWidth = maxWidth
    Box(
      modifier = Modifier
        .fillMaxHeight()
        .width(sliderWidth * VALUE)
        .background(ACTIVE_COLOR)
       )
  }
}