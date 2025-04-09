package org.just_somebody.pocket_pixel.searchScreen.presentation

import org.just_somebody.pocket_pixel.core.Game

data class SearchState(
  val errorMessage  : String?     = null,
  val searchQuery   : String      = "",
  val searchResults : List<Game>  = listOf(
    Game(
      releaseYear = 1985,
      title = "The Legend of Zelda",
      publisher = "Capcom",
      description = "Wake up at the beach, beware the racoon",
      imageUrl = "https://upload.wikimedia.org/wikipedia/en/c/c1/Link%27s_Awakening.png"
    ),
    Game(
      releaseYear = 1985,
      title = "Pokemon Red",
      publisher = "Nintendo",
      description = "Gotta catch em all",
      imageUrl = "https://upload.wikimedia.org/wikipedia/en/c/c1/Link%27s_Awakening.png"
    )
  )
)