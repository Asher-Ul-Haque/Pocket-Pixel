package org.just_somebody.pocket_pixel.searchScreen.presentation

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls
import org.just_somebody.pocket_pixel.favoritesScreen.presentation.FavoriteActions
import org.just_somebody.pocket_pixel.favoritesScreen.presentation.FavoritesState

class SearchViewModel : ViewModel()
{
  val state = MutableStateFlow(FavoritesState())

  fun onAction(ACTION : FavoriteActions)
  {
    when (ACTION)
    {
      is FavoriteActions.ChangeFavoriteTerm ->
        state.update { newState -> newState.copy(searchQuery = ACTION.SEARCH_TERM) }

      is FavoriteActions.Favorite ->
        viewModelScope.launch ()
        {
          getSplashNetworkCalls().searchGames(state.value.searchQuery)
            .onSuccess { result ->   state.update { newState -> newState.copy(searchResults = result) } }
            .onError   { error  ->   state.update { newState -> newState.copy(searchResults = emptyList(), errorMessage = error.toString()) } }
        }

      is FavoriteActions.GoToGame ->
        {

        }
    }
  }
}