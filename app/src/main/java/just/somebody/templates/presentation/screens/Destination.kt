package just.somebody.templates.presentation.screens

import kotlinx.serialization.Serializable


sealed interface Destination
{
  @Serializable data object GraphA  : Destination
  @Serializable data object GraphB  : Destination
  @Serializable data object ScreenA : Destination
  @Serializable data object ScreenB : Destination
  @Serializable data class  ArgScreen(val ID : Int) : Destination
}