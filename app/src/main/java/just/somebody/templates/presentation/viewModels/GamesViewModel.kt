package just.somebody.templates.presentation.viewModels

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.repositories.GameRepository
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.flatMapLatest
import kotlinx.coroutines.flow.flowOf
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import java.lang.Thread.State

class GamesViewModel(private val REPO : GameRepository) : ViewModel()
{

  private val _selectedGame : MutableStateFlow<Game?> = MutableStateFlow<Game?>(null)
  public  val selectedGame  : StateFlow<Game?>        = _selectedGame

  val favoriteGames : StateFlow<List<Game>> =
    REPO
      .getFavoriteGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList())

  val recentlyPlayedGames : StateFlow<List<Game>> =
    REPO
      .getRecentlyPlayedGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList()
      )

  val newGames : StateFlow<List<Game>> =
    REPO
      .getNeverPlayedGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList()
      )

  private val _searchQuery : MutableStateFlow<String> = MutableStateFlow("")
  public  val searchQuery  : StateFlow<String>        = _searchQuery
  val searchResults: StateFlow<List<Game>> =
    _searchQuery
      .debounce(300)
      .flatMapLatest ()
      { query ->
        if (query.isBlank()) REPO.getAllGames()
        else                 REPO.searchGames(query)
      }
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList()
      )


  fun updateSearchQuery(QUERY : String)
  { _searchQuery.value = QUERY.trim() }

  fun toggleFavorite(GAME : Game)
  {
    viewModelScope.launch { REPO.updateGame(GAME.copy(isFavorite = !GAME.isFavorite)) }
  }

  fun markAsPlayed(GAME : Game)
  {
    viewModelScope.launch { REPO.updateLastPlayed(GAME.id, System.currentTimeMillis()) }
  }

  fun detectAndInsertRoms()
  {
    viewModelScope.launch ()
    {
      val externalStorageManager = App.appModule.externalStorageManager
      val key = "GAME_BOY_ROMS"
      if (externalStorageManager.getDirectory(key) == null) return@launch
      val repo = App.appModule.repo
      repo.insertGames(key)
    }
  }

  fun selectGame(GAME : Game?)
  {
    viewModelScope.launch { _selectedGame.emit(GAME) }
  }
}