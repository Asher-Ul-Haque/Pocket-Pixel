package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.minimumInteractiveComponentSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
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
  ON_CLICK : () -> Unit = {},
  MODIFIER : Modifier   = Modifier,
  CONTENT  : @Composable () -> Unit = {},
)
{
  val scope = rememberCoroutineScope()
  Box(
    modifier = MODIFIER
      .minimumInteractiveComponentSize()
      .shadow(
        elevation    = 4.dp,
        shape        = RectangleShape,
        ambientColor = Color.Black,
        spotColor    = Color.Black
      )
      .padding(8.dp)
      .background(GameBoyColors.MediumGreen)
      .border(4.dp, GameBoyColors.Green, RectangleShape)
      .clickable(
        onClick =
        {
          scope.launch { SoundController.playSound(SoundEffect.Click) }
          ON_CLICK()
        }
      ),
    contentAlignment = Alignment.Center
  )
  { CONTENT(); }
}