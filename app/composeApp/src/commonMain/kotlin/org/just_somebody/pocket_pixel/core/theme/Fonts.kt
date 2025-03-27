package org.just_somebody.pocket_pixel.core.theme

import androidx.compose.material3.Typography
import androidx.compose.runtime.Composable
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import org.jetbrains.compose.resources.ExperimentalResourceApi
import org.jetbrains.compose.resources.Font
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.pokemon_fire_red

@OptIn(ExperimentalResourceApi::class)
@Composable
fun PokeFontFamily() =
  FontFamily(
    Font(Res.font.pokemon_fire_red, weight = FontWeight.Light),
    Font(Res.font.pokemon_fire_red, weight = FontWeight.Normal),
    Font(Res.font.pokemon_fire_red, weight = FontWeight.Medium),
    Font(Res.font.pokemon_fire_red, weight = FontWeight.SemiBold),
    Font(Res.font.pokemon_fire_red, weight = FontWeight.Bold),
    Font(Res.font.pokemon_fire_red, weight = FontWeight.ExtraBold)
  )

@Composable
fun PokeTypography() =
  Typography().run ()
  {
    val fontFamily = PokeFontFamily();
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