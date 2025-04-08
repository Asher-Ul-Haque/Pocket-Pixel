package org.just_somebody.pocket_pixel.searchScreen.presentation

import org.just_somebody.pocket_pixel.core.Game

data class SearchState(
  val errorMessage  : String?     = null,
  val searchQuery   : String      = "Pokemon Red",
  val searchResults : List<Game>  = emptyList()
)