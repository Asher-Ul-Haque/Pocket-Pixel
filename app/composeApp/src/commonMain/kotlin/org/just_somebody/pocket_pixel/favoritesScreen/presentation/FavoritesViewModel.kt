package org.just_somebody.pocket_pixel.favoritesScreen.presentation

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.Gamer
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls

class FavoritesViewModel(private val GAMER : Gamer) : ViewModel()
{
  val state = MutableStateFlow(FavoritesState())

  init { onAction(FavoriteActions.GetFavorites) }

  fun onAction(ACTION : FavoriteActions)
  {
    when (ACTION)
    {
      is FavoriteActions.GetFavorites ->
        viewModelScope.launch ()
        {
          getSplashNetworkCalls().getFavoriteGames(GAMER)
            .onSuccess  { result  -> state.update { newState -> newState.copy(favoritesResult = result) } }
            .onError    { error   -> state.update { newState -> newState.copy(favoritesResult = emptyList(), errorMessage = error.toString()) } }
        }


      is FavoriteActions.GoToGame ->
        {

        }
    }
  }
}