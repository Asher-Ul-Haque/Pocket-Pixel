package org.just_somebody.pocket_pixel.core

import androidx.compose.runtime.Composable

@Composable
expect fun getWindowWidth() : Int;

@Composable
expect fun getWindowHeight() : Int;

@Composable
fun isLandscape() : Boolean
{
  return getWindowWidth() > getWindowHeight()
}