package org.just_somebody.pocket_pixel.splashScreen.presentation

import org.just_somebody.pocket_pixel.core.Gamer
import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls
import org.just_somebody.pocket_pixel.core.networking.NetworkCalls

data class SplashState(
  val gamer           : Gamer                     = Gamer("JustSomebody", "password"),
  val isValidName     : Boolean                   = false,
  val isValidPassword : Boolean                   = false,
  val isLoggingIn     : Boolean                   = false,
  val isLoggedIn      : Boolean                   = false,
  val isLoginError    : Boolean                   = false,
  val networkCalls    : NetworkCalls              = getSplashNetworkCalls()
)