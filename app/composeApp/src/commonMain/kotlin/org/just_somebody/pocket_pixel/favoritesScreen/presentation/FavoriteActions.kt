package org.just_somebody.pocket_pixel.favoritesScreen.presentation

import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.core.Gamer


sealed interface FavoriteActions
{
  data class  GetFavorites    (val GAMER : Gamer)         : FavoriteActions
  data class  GoToGame        (val GAME : Game)           : FavoriteActions
  data class  ChangeSearchTerm(val SERACH_TERM : String)  : FavoriteActions
  data object Filter                                      : FavoriteActions
}