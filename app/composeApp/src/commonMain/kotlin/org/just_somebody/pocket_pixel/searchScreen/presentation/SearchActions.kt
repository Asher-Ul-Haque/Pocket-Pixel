package org.just_somebody.pocket_pixel.searchScreen.presentation

import org.just_somebody.pocket_pixel.core.Game


sealed interface SearchActions
{
  data class  ChangeSearchTerm (val SEARCH_TERM : String) : SearchActions
  data object Search                                      : SearchActions
  data class  GoToGame (val GAME : Game)                  : SearchActions
}