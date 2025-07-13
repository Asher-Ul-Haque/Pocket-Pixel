package just.somebody.templates.ui.theme

import androidx.compose.ui.graphics.Color

data object MonoChrome
{
  val Blue90 = Color(0xFFE3F2FD)
  val Blue70 = Color(0xFF90CAF9)
  val Blue50 = Color(0xFF2196F3)
  val Blue30 = Color(0xFF1976D2)
  val Blue10 = Color(0xFF0D47A1)
}

data object Analogous
{
  val GreenBlue80  = Color(0xFF80DEEA)
  val GreenBlue60  = Color(0xFF26C6DA)
  val PurpleBlue80 = Color(0xFFB39DDB)
  val PurpleBlue60 = Color(0xFF9575CD)
}

data object Complementary
{
  val OrangeLight   = Color(0xFFFFCC80)
  val OrangePrimary = Color(0xFFFF9800)
  val OrangeDark    = Color(0xFFF57C00)
  val BluePrimary   = MonoChrome.Blue50
  val BlueDark      = MonoChrome.Blue30
}

data object Triadic
{
  val YellowPrimary  = Color(0xFFFFEB3B)
  val MagentaPrimary = Color(0xFFE91E63)
  val CyanPrimary    = Color(0xFF00BCD4)
}

data object SplitComplementary
{
  val RedPrimary       = Color(0xFFF44336)
  val BlueGreenPrimary = Color(0xFF009688)
  val BlueGreenLight   = Color(0xFF4DB6AC)
}

data object Neutral
{
  val Grey90 = Color(0xFFF5F5F5)
  val Grey70 = Color(0xFFBDBDBD)
  val Grey50 = Color(0xFF757575)
  val Grey30 = Color(0xFF424242)
  val Grey10 = Color(0xFF212121)
}

data object GameBoyColors
{
  val DarkGreen    : Color = Color(0xff0f380f);
  val MediumGreen  : Color = Color(0xff306230);
  val Green        : Color = Color(0xff8bac0f);
  val LightGreen   : Color = Color(0xff9bbc0f);
  val Error        : Color = Color(0xFF76393f);
}