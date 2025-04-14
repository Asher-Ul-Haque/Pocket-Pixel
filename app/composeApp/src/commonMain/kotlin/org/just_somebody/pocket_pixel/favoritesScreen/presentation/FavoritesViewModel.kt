package org.just_somebody.pocket_pixel.favoritesScreen.presentation

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getNetworkCalls
import org.just_somebody.pocket_pixel.depInj.setGame

class FavoritesViewModel() : ViewModel()
{
  val state = MutableStateFlow(FavoritesState())

  fun onAction(ACTION : FavoriteActions)
  {
    when (ACTION)
    {
      is FavoriteActions.GetFavorites ->
        viewModelScope.launch ()
        {
          getNetworkCalls().getFavoriteGames(ACTION.GAMER)
            .onSuccess  { result  -> state.update { newState -> newState.copy(favoritesResult = result) } }
            .onError    { error   -> state.update { newState -> newState.copy(favoritesResult = emptyList(), errorMessage = error.toString()) } }
        }

      is FavoriteActions.Filter ->
        {
          val lowerQuery  = state.value.searchQuery
          val games       = state.value.favoritesResult
          state.update()
          { newState ->
            newState.copy(
              filteredResults = games.filter()
              { game ->
                game.title.contains       (lowerQuery, ignoreCase = true) ||
                game.publisher.contains   (lowerQuery, ignoreCase = true) ||
                game.description.contains (lowerQuery, ignoreCase = true)
              }
            )
          }
        }

      is FavoriteActions.ChangeSearchTerm ->
        { state.update { newState -> newState.copy(searchQuery = ACTION.SERACH_TERM) } }

      is FavoriteActions.GoToGame -> setGame(ACTION.GAME)
    }
  }
}