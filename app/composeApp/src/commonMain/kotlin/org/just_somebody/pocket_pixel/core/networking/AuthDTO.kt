package org.just_somebody.pocket_pixel.core.networking

import kotlinx.serialization.Serializable

@Serializable
data class Request(
  val gamerTag : String,
  val password : String
)

@Serializable
data class Response(
  val token     : String,
  val gamerID   : Long,
  val gamerTag  : String
)

@Serializable
data class MessageResponse (val message : String)