package just.somebody.templates.presentation.screens

import just.somebody.templates.domain.models.Game
import kotlinx.serialization.Serializable


sealed interface Destination
{
  @Serializable data object Home      : Destination
  @Serializable data object Favorites : Destination
  @Serializable data object Server    : Destination
  @Serializable data object Search    : Destination
  @Serializable data object Settings  : Destination
  @Serializable data class  Emulator(val URI : String)  : Destination
}