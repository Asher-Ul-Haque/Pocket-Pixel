package org.just_somebody.pocket_pixel.favoritesScreen.presentation

import org.just_somebody.pocket_pixel.core.Game


sealed interface FavoriteActions
{
  data object GetFavorites                                  : FavoriteActions
  data class  GoToGame (val GAME : Game)                    : FavoriteActions
}