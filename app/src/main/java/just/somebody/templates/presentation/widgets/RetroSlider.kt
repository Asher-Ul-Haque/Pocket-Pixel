package just.somebody.templates.presentation.widgets

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

@Composable
fun RetroSlider(
    VALUE           : Float,
    ON_VALUE_CHANGE : (Float) -> Unit,
    MODIFIER        : Modifier = Modifier,
    ACTIVE_COLOR    : Color = GameBoyColors.Green,
    INACTIVE_COLOR  : Color = GameBoyColors.DarkGreen,
    BORDER_COLOR    : Color = GameBoyColors.Green
) {
    BoxWithConstraints(
        modifier = MODIFIER
            .height(24.dp)
            .border(2.dp, BORDER_COLOR, RectangleShape)
            .background(INACTIVE_COLOR)
            .pointerInput(Unit) {
                detectTapGestures { offset ->
                    ON_VALUE_CHANGE((offset.x / size.width).coerceIn(0f, 1f))
                }
            }
            .pointerInput(Unit) {
                awaitPointerEventScope {
                    while (true) {
                        val event = awaitPointerEvent()
                        if (event.changes.any { it.pressed }) {
                            val position = event.changes.first().position
                            ON_VALUE_CHANGE((position.x / size.width).coerceIn(0f, 1f))
                        }
                    }
                }
            }
    ) {
        val sliderWidth = maxWidth
        Box(
            modifier = Modifier
                .fillMaxHeight()
                .width(sliderWidth * VALUE)
                .background(ACTIVE_COLOR)
        )
    }
}
