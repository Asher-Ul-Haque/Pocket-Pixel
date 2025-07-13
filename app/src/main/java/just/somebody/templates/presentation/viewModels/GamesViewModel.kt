package just.somebody.templates.presentation.viewModels

import android.net.Uri
import androidx.compose.runtime.mutableStateMapOf
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import io.ktor.client.request.request
import io.ktor.http.isSuccess
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.appModule.network.NetworkResult
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.repositories.GameRepository
import just.somebody.templates.presentation.screens.Destination
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.distinctUntilChanged
import kotlinx.coroutines.flow.flatMapLatest
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

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
    viewModelScope.launch ()
    {
      REPO.updateLastPlayed(GAME.id, System.currentTimeMillis())
      App.appModule.navigator.navigate(Destination.Emulator(GAME.romUri))
    }
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

  private val boxArtFetcher = App.appModule.boxArtFetcher

  private val _boxArtMap : MutableStateFlow<Map<String, String?>> = MutableStateFlow<Map<String, String?>>(emptyMap())
  public  val boxArtMap  : StateFlow<Map<String, String?>>        = _boxArtMap

  fun getBoxArtFlow(title: String): Flow<String?>
  {
    if (!_boxArtMap.value.containsKey(title)) fetchBoxArt(title)
    return boxArtMap.map { it[title] }
  }

  init { observeInternetConnectivity() }

  private fun observeInternetConnectivity()
  {
    viewModelScope.launch ()
    {
      App.appModule.hardwareManager.isConnectedToInternet
        .distinctUntilChanged()
        .collect ()
        { status ->
          if (status is NetworkStatus.Available) retryMissingBoxArts()
        }
    }
  }

  private fun retryMissingBoxArts()
  {
    val missingGames = _boxArtMap.value
      .filter { (_, url) -> url == null }
      .keys

    for (title in missingGames) { fetchBoxArt(title) }
  }

  private fun fetchBoxArt(title : String)
  {
    viewModelScope.launch ()
    {
      boxArtFetcher
        .fetchBoxArt(title)
        .collect ()
        { url ->
          _boxArtMap.update()
          { old -> old + (title to url) }
        }
    }
  }
}