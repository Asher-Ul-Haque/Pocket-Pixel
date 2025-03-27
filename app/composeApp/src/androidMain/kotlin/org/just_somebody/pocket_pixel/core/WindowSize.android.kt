package org.just_somebody.pocket_pixel.core

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalConfiguration

@Composable
actual fun getWindowWidth(): Int
{
  val config = LocalConfiguration.current
  return config.screenWidthDp
}

@Composable
actual fun getWindowHeight(): Int
{
  val config = LocalConfiguration.current
  return config.screenHeightDp
}