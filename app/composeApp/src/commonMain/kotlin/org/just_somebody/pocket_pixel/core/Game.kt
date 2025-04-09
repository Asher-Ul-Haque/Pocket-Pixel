package org.just_somebody.pocket_pixel.core

import kotlinx.serialization.Serializable

@Serializable
data class Game
(
  val releaseYear : Int,
  val title       : String,
  val publisher   : String,
  val description : String,
  val imageUrl    : String
);