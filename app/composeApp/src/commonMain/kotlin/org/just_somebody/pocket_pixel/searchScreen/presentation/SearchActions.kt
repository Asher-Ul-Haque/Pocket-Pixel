package org.just_somebody.pocket_pixel.searchScreen.presentation

import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.favoritesScreen.presentation.FavoriteActions


sealed interface SearchActions
{
  data class  ChangeFavoriteTerm (val SEARCH_TERM : String) : FavoriteActions
  data object Favorite                                      : FavoriteActions
  data class  GoToGame (val GAME : Game)                  : FavoriteActions
}