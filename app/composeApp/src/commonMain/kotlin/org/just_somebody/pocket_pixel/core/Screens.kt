package org.just_somebody.pocket_pixel.core

import kotlinx.serialization.Serializable

sealed interface Screens
{
  @Serializable
  data object SplashScreen   : Screens

  @Serializable
  data object MainScreen     : Screens

  @Serializable
  data object MePage         : Screens

  @Serializable
  data object ExplorePage    : Screens

  @Serializable
  data object FavoritesPage  : Screens

  @Serializable
  data object SearchPage     : Screens

  @Serializable
  data object DownloadsPage  : Screens
}