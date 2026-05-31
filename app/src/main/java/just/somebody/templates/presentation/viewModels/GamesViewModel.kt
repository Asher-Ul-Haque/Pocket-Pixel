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
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.FlowPreview
import kotlinx.coroutines.channels.Channel
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
import java.util.Collections

class GamesViewModel(private val REPO : GameRepository) : ViewModel()
{

  private val _selectedGame : MutableStateFlow<Game?> = MutableStateFlow<Game?>(null)
  public  val selectedGame  : StateFlow<Game?>        = _selectedGame
  private var networkStatus : NetworkStatus           = NetworkStatus.Unavailable

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
  @OptIn(FlowPreview::class, ExperimentalCoroutinesApi::class)
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

  fun updateBoxArtUrl(GAME: Game, URL: String)
  {
    viewModelScope.launch ()
    {
      REPO.updateGame(GAME.copy(boxArtUrl = URL))
      _selectedGame.emit(null)
    }
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
      val key                     = "GAME_BOY_ROMS"
      val repo                    = App.appModule.repo
      repo.syncGamesWithStorage(key)
    }
  }

  fun selectGame(GAME : Game?)
  {
    viewModelScope.launch { _selectedGame.emit(GAME) }
  }

  private val boxArtFetcher = App.appModule.boxArtFetcher

  private val _boxArtMap  : MutableStateFlow<Map<String, String?>> = MutableStateFlow(emptyMap())
  val boxArtMap           : StateFlow<Map<String, String?>> = _boxArtMap

  private val boxArtQueue  = Channel<String>(capacity = Channel.UNLIMITED)
  private val queuedTitles = Collections.synchronizedSet(mutableSetOf<String>())


  fun getBoxArtFlow(game: Game): Flow<String?>
  {
    if (game.boxArtUrl != null) return flow { emit(game.boxArtUrl) }

    val title = game.title
    if (!_boxArtMap.value.containsKey(title) && networkStatus == NetworkStatus.Available) fetchBoxArt(game)
    return boxArtMap.map { it[title] }
  }

  init
  {
    observeInternetConnectivity()
    startBoxArtWorker()
    detectAndInsertRoms()
  }

  private fun observeInternetConnectivity()
  {
    viewModelScope.launch ()
    {
      App.appModule.hardwareManager.isConnectedToInternet
        .collect ()
        { status ->
          networkStatus = status
          if (status is NetworkStatus.Available) retryMissingBoxArts()
        }
    }
  }

  private fun retryMissingBoxArts()
  {
    // This logic might need refinement since we now have DB persistence,
    // but for now let's keep it simple.
  }

  private fun fetchBoxArt(game: Game)
  {
    val title = game.title
    if (_boxArtMap.value.containsKey(title)) return
    if (queuedTitles.contains(title)) return

    _boxArtMap.update { it + (title to null) }
    queuedTitles.add(title)

    viewModelScope.launch { boxArtQueue.send(title) }
  }

  private fun startBoxArtWorker()
  {
    viewModelScope.launch(Dispatchers.Default)
    {
      for (title in boxArtQueue)
      {
        try
        {
          boxArtFetcher.fetchBoxArt(title).collect()
          { url ->
            _boxArtMap.update { it + (title to url) }
            if (url != null)
            {
              // Cache to DB if found
              REPO.getGameByTitle(title)?.let { game ->
                if (game.boxArtUrl == null) {
                   REPO.updateGame(game.copy(boxArtUrl = url))
                }
              }
            }
          }
        }
        catch (e: Exception)
        {
          ForgeLogger.error("Failed to fetch box art for $title: $e")
          _boxArtMap.update { it + (title to null) }
        }
        finally { queuedTitles.remove(title) }
      }
    }
  }
}