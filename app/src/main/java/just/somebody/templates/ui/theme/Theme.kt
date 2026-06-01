package just.somebody.templates.ui.theme

import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext

/** Optimized Material3 light color spectrum configured using classic dot-matrix green matrix shades. */
val GameBoyLightColorScheme: ColorScheme = lightColorScheme(
  primary        = GameBoyColors.MediumGreen,
  onPrimary      = GameBoyColors.LightGreen,
  secondary      = GameBoyColors.Green,
  onSecondary    = GameBoyColors.DarkGreen,
  background     = GameBoyColors.LightGreen,
  onBackground   = GameBoyColors.DarkGreen,
  surface        = GameBoyColors.Green,
  onSurface      = GameBoyColors.DarkGreen,
  error          = GameBoyColors.Error,
  onError        = Color.White)

/** Optimized Material3 dark color spectrum inversion using localized high-contrast green tones. */
val GameBoyDarkColorScheme: ColorScheme = darkColorScheme(
  primary        = GameBoyColors.LightGreen,
  onPrimary      = GameBoyColors.DarkGreen,
  secondary      = GameBoyColors.Green,
  onSecondary    = GameBoyColors.DarkGreen,
  background     = GameBoyColors.DarkGreen,
  onBackground   = GameBoyColors.LightGreen,
  surface        = GameBoyColors.MediumGreen,
  onSurface      = GameBoyColors.LightGreen,
  error          = GameBoyColors.Error,
  onError        = Color.White
                                                         )

/**
 * Main application layout theme wrapper establishing systemic styling configurations across Composable subtrees.
 *
 * Couples customized retro monochromatic design parameters to standard Material3 token keys, ensuring
 * a unified presentation feel. Note: While dynamic color parameters are specified, options are hard-coded
 * internally to maintain the game console aesthetic integrity.
 *
 * @param darkTheme System flag overriding or identifying current dark state visualization rules.
 * @param dynamicColor Configuration selector targeting Android 12+ dynamic wallpaper engine properties.
 * @param content Target Composable functional structure layout evaluated within this theme wrapper.
 */
@Composable
fun TemplateTheme(
  darkTheme     : Boolean                 = isSystemInDarkTheme(),
  dynamicColor  : Boolean                 = true,
  content       : @Composable () -> Unit)
{
  val colorScheme = when
  {
    dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S ->
    {
      val context = LocalContext.current

      if (darkTheme)  GameBoyDarkColorScheme
      else            GameBoyLightColorScheme
    }
    darkTheme -> GameBoyDarkColorScheme
    else      -> GameBoyLightColorScheme
  }

  MaterialTheme(
    colorScheme = colorScheme,
    typography  = MinecraftTypography,
    content     = content)
}