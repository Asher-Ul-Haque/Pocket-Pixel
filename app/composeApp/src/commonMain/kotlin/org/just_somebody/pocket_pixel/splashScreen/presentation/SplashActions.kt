package org.just_somebody.pocket_pixel.splashScreen.presentation

sealed interface SplashActions
{
  data class  ChangeName (val NAME: String) : SplashActions
  data class  ChangePass (val PASS: String) : SplashActions
  data object Register                      : SplashActions
  data object Login                         : SplashActions
  data object AutoLogin                     : SplashActions
}
