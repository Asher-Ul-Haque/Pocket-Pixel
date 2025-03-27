package org.just_somebody.pocket_pixel.splashScreen.domain

import kotlinx.serialization.Serializable

@Serializable
data class Gamer(
  val name      : String,
  val password  : String,
)
