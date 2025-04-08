package org.just_somebody.pocket_pixel.searchScreen.presentation

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls

class SearchViewModel : ViewModel()
{
  var state by mutableStateOf(SearchState())
    private set;

  fun onAction(ACTION : SearchActions)
  {
    when (ACTION)
    {
      is SearchActions.ChangeSearchTerm ->
        state = state.copy(searchQuery = ACTION.SEARCH_TERM)

      is SearchActions.Search ->
        viewModelScope.launch ()
        {
          getSplashNetworkCalls().searchGames(state.searchQuery)
            .onSuccess { result -> state = state.copy(searchResults = result) }
            .onError   { error  -> state = state.copy(searchResults = emptyList(), errorMessage = error.toString() )  }
        }
    }
  }
}