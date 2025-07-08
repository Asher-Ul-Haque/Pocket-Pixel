package just.somebody.pocketpixel.ui.theme

import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import just.somebody.templates.ui.theme.ExampleTypography
import just.somebody.templates.ui.theme.MonoChrome

private val DarkColorScheme = darkColorScheme(
  primary   = MonoChrome.Blue90,
  secondary = MonoChrome.Blue50,
  tertiary  = MonoChrome.Blue10
)

private val LightColorScheme = lightColorScheme(
  primary   = Color.Black,
  secondary = MonoChrome.Blue70,
  tertiary  = MonoChrome.Blue30
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

      if (darkTheme)  dynamicDarkColorScheme(context)
      else            dynamicLightColorScheme(context)
    }
    darkTheme -> DarkColorScheme
    else      -> LightColorScheme
  }

  MaterialTheme(
    colorScheme = colorScheme,
    typography  = ExampleTypography,
    content     = content
  )
}