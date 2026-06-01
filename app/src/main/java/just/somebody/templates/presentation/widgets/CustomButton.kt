package just.somebody.templates.presentation.widgets

import androidx.compose.animation.core.animateDpAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.minimumInteractiveComponentSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.unit.dp
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.launch


@Composable
fun CustomButton(
  ON_CLICK : () -> Unit             = {},
  MODIFIER : Modifier               = Modifier,
  COLOR    : Color                  = GameBoyColors.MediumGreen,
  CONTENT  : @Composable () -> Unit = {})
{
  val scope             = rememberCoroutineScope()
  val interactionSource = remember { MutableInteractionSource() }
  val isPressed by interactionSource.collectIsPressedAsState()
  val elevation by animateDpAsState(
    if (isPressed)  0.dp
    else            4.dp,
    label = "elevation")
  val offset    by animateDpAsState(
    if (isPressed)  2.dp
    else            0.dp,
    label = "offset")

  Box(
    modifier = MODIFIER
      .minimumInteractiveComponentSize()
      .clickable(
        interactionSource = interactionSource,
        indication        = null,
        onClick           =
        {
          scope.launch { SoundController.playSound(SoundEffect.Click) }
          ON_CLICK()
        }
      )
      .offset(x = offset, y = offset)
      .shadow(
        elevation    = elevation,
        shape        = RectangleShape,
        ambientColor = Color.Black,
        spotColor    = Color.Black
      )
      .padding(8.dp)
      .background(COLOR)
      .border(4.dp, GameBoyColors.Green, RectangleShape),
    contentAlignment = Alignment.Center
  )
  { CONTENT(); }
}
