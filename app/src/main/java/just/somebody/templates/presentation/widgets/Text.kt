package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily

/**
 * A customized text view wrapper that handles application typography styling.
 *
 * Automatically injects the global monospaced retro typeface scheme while configuring standard line boundaries
 * and fallback overflow truncated metrics. It includes an embedded padding fallback assignment rule that isolates
 * standalone inline strings with balanced margin space when a custom structural layout modifier is omitted.
 *
 * @param TEXT The character sequence data to be drawn on the display layer.
 * @param MODIFIER [Modifier] used to establish positional layout logic or override default padding constraints.
 * @param COLOR The color style applied to the text glyphs. Defaults to [GameBoyColors.LightGreen].
 * @param FONT_SIZE The scale dimension unit tracking text height and scale parameters. Defaults to `16`.
 * @param MAX_LINES Hard rendering threshold limit capping how many vertical lines the text block may occupy before clipping. Defaults to `100`.
 */
@Composable
fun CustomText(
  TEXT      : String,
  MODIFIER  : Modifier = Modifier,
  COLOR     : Color    = GameBoyColors.LightGreen,
  FONT_SIZE : Int      = 16,
  MAX_LINES : Int      = 100)
{
  Text(
    modifier   = if (MODIFIER == Modifier) MODIFIER.padding(16.dp)
    else                      MODIFIER,
    text       = TEXT,
    color      = COLOR,
    fontSize   = FONT_SIZE.sp,
    fontFamily = MinecraftFontFamily,
    overflow   = TextOverflow.Ellipsis,
    maxLines   = MAX_LINES)
}