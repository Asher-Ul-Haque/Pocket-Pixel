package just.somebody.pocketpixel.ui.theme

import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.ColorScheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import just.somebody.templates.ui.theme.PokeTypography
import just.somebody.templates.ui.theme.GameBoyColors


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
  onError        = Color.White
)

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


@Composable
fun TemplateTheme(
  darkTheme     : Boolean                 = isSystemInDarkTheme(),
  dynamicColor  : Boolean                 = true,
  content       : @Composable () -> Unit
)
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
    typography  = PokeTypography,
    content     = content
  )
}