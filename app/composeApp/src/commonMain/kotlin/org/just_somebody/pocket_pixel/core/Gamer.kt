package org.just_somebody.pocket_pixel.core

import kotlinx.serialization.Serializable

@Serializable
data class Gamer(
  val name      : String,
  val password  : String,
)
