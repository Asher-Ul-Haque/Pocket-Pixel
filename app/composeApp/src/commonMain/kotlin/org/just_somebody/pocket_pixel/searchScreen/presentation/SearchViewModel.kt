package org.just_somebody.pocket_pixel.searchScreen.presentation

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getNetworkCalls
import org.just_somebody.pocket_pixel.depInj.setGame

class SearchViewModel : ViewModel()
{
  val state = MutableStateFlow(SearchState())

  fun onAction(ACTION : SearchActions)
  {
    when (ACTION)
    {
      is SearchActions.ChangeSearchTerm ->
        state.update { newState -> newState.copy(searchQuery = ACTION.SEARCH_TERM) }

      is SearchActions.Search ->
        viewModelScope.launch ()
        {
          getNetworkCalls().searchGames(state.value.searchQuery)
            .onSuccess { result ->   state.update { newState -> newState.copy(searchResults = result) } }
            .onError   { error  ->   state.update { newState -> newState.copy(searchResults = emptyList(), errorMessage = error.toString()) } }
        }

      is SearchActions.GoToGame -> setGame(ACTION.GAME)
    }
  }
}