package org.just_somebody.pocket_pixel.splashScreen.presentation

import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls
import org.just_somebody.pocket_pixel.splashScreen.data.SplashScreenNetworkCalls
import org.just_somebody.pocket_pixel.splashScreen.domain.Gamer

data class SplashState(
  val gamer           : Gamer                     = Gamer("", ""),
  val isValidName     : Boolean                   = false,
  val isValidPassword : Boolean                   = false,
  val isLoggingIn     : Boolean                   = false,
  val isLoggedIn      : Boolean                   = false,
  val isLoginError    : Boolean                   = false,
  val networkCalls    : SplashScreenNetworkCalls  = getSplashNetworkCalls()
)