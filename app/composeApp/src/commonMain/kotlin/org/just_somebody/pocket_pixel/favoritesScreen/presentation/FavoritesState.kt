package org.just_somebody.pocket_pixel.favoritesScreen.presentation

import org.just_somebody.pocket_pixel.core.Game

data class FavoritesState(
  val errorMessage    : String?     = null,
  val searchQuery     : String      = "",
  val filteredResults : List<Game>  = emptyList(),
  val favoritesResult : List<Game>  = emptyList()
)