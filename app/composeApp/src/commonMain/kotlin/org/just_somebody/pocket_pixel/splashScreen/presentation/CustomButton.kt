import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.minimumInteractiveComponentSize
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier

import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors

@Composable
fun CustomButton(
  ON_CLICK : () -> Unit = {},
  CONTENT  : @Composable () -> Unit = {},
  MODIFIER : Modifier   = Modifier,
  SIZE     : Dp = 200.dp)
{
  Box(
    modifier = MODIFIER
      .minimumInteractiveComponentSize()
      .padding(8.dp)
      .background(GameBoyColors.MediumGreen)
      .border(4.dp, GameBoyColors.Green, RectangleShape)
      .clickable(onClick = ON_CLICK),
    contentAlignment = Alignment.Center
  )
  {
    CONTENT();
  }
}
