package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.PokeFontFamily

@Composable
fun CustomText(
  TEXT      : String,
  MODIFIER  : Modifier = Modifier,
  COLOR     : Color    = GameBoyColors.LightGreen,
  FONT_SIZE : Int      = 32)
{
  Text(
    modifier   = MODIFIER.padding(16.dp),
    text       = TEXT,
    color      = COLOR,
    fontSize   = FONT_SIZE.sp,
    fontFamily = PokeFontFamily,
  )
}