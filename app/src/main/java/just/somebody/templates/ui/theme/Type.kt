package just.somebody.templates.ui.theme

import androidx.compose.material3.Typography
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import just.somebody.templates.R

val PokeFontFamily : FontFamily =
  FontFamily(
    Font(R.font.pokemon_fire_red, weight = FontWeight.Light),
    Font(R.font.pokemon_fire_red, weight = FontWeight.Normal),
    Font(R.font.pokemon_fire_red, weight = FontWeight.Medium),
    Font(R.font.pokemon_fire_red, weight = FontWeight.SemiBold),
    Font(R.font.pokemon_fire_red, weight = FontWeight.Bold),
    Font(R.font.pokemon_fire_red, weight = FontWeight.ExtraBold)
  )

val PokeTypography : Typography =
  Typography().run ()
  {
    val fontFamily = PokeFontFamily
    copy(
      displayLarge      = displayLarge.copy  (fontFamily = fontFamily),
      displayMedium     = displayMedium.copy (fontFamily = fontFamily),
      displaySmall      = displayLarge.copy  (fontFamily = fontFamily),

      headlineLarge     = headlineLarge.copy (fontFamily = fontFamily),
      headlineMedium    = headlineMedium.copy(fontFamily = fontFamily),
      headlineSmall     = headlineSmall.copy (fontFamily = fontFamily),

      titleLarge        = titleLarge.copy    (fontFamily = fontFamily),
      titleMedium       = titleMedium.copy   (fontFamily = fontFamily),
      titleSmall        = titleSmall.copy    (fontFamily = fontFamily),

      bodyLarge         = bodyLarge.copy     (fontFamily = fontFamily),
      bodyMedium        = bodyMedium.copy    (fontFamily = fontFamily),
      bodySmall         = bodySmall.copy     (fontFamily = fontFamily),

      labelLarge        = labelLarge.copy    (fontFamily = fontFamily),
      labelMedium       = labelMedium.copy   (fontFamily = fontFamily),
      labelSmall        = labelSmall.copy    (fontFamily = fontFamily)
    )
  }