package org.just_somebody.pocket_pixel.searchScreen.presentation


sealed interface SearchActions
{
  data class  ChangeSearchTerm (val SEARCH_TERM : String) : SearchActions
  data object Search                                      : SearchActions
}