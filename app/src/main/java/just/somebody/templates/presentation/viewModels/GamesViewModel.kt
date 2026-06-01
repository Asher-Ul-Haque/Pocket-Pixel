package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
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

/**
 * Presentation layer component coordinating local catalog data queries and remote asset updates.
 * Implements a sequential Channel execution routine to scrape, download, and store promotional illustrations,
 * paired with debounce-driven text fuzzy matching logic for local database searches.
 *
 * @property REREPO Domain abstraction data driver routing transactions for item metadata rows.
 */
class GamesViewModel(private val REPO : GameRepository) : ViewModel()
{
  /** Updates the local settings layer flag verifying feedback metric fulfillment. */
  fun markAsRated()
  {
    viewModelScope.launch()
    {
      val current = App.appModule.dataStoreManager.getSettings()
      App.appModule.dataStoreManager.updateSettings(current.copy(hasRated = true))
    }
  }

  private val _selectedGame : MutableStateFlow<Game?> = MutableStateFlow<Game?>(null)
  public  val selectedGame  : StateFlow<Game?>        = _selectedGame
  private var networkStatus : NetworkStatus           = NetworkStatus.Unavailable

  /** Hot flow broadcasting continuous tracking lists for prioritized favorited items. */
  val favoriteGames : StateFlow<List<Game>> =
    REPO
      .getFavoriteGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList())

  /** Hot flow broadcasting localized historical tracking listings sorted descending by access ticks. */
  val recentlyPlayedGames : StateFlow<List<Game>> =
    REPO
      .getRecentlyPlayedGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList())

  /** Hot flow broadcasting elements filtering catalogued records that carry no historical usage flags. */
  val newGames : StateFlow<List<Game>> =
    REPO
      .getNeverPlayedGames()
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList())

  private val _searchQuery : MutableStateFlow<String> = MutableStateFlow("")
  public  val searchQuery  : StateFlow<String>        = _searchQuery

  /**
   * Evaluates text inputs inside a debounced reactive pipeline to emit matches matching character queries.
   */
  @OptIn(FlowPreview::class, ExperimentalCoroutinesApi::class)
  val searchResults: StateFlow<List<Game>> =
    _searchQuery
      .debounce(300)
      .distinctUntilChanged()
      .flatMapLatest()
      { query ->
        if (query.isBlank())
        { REPO.getAllGames() }
        else
        {
          REPO.getAllGames().map()
          { allGames ->
            allGames.filter()
            { game -> fuzzyMatch(game.title, query) }.sortedByDescending { it.lastPlayed ?: 0L }
          }
        }
      }
      .stateIn(
        viewModelScope,
        SharingStarted.WhileSubscribed(5000),
        emptyList())

  /**
   * Loops through sequential text components to verify char alignment order maps.
   *
   * @param TEXT Raw resource string being validated.
   * @param QUERY User search criteria term.
   * @return Boolean flag confirming full character sequential presence.
   */
  private fun fuzzyMatch(TEXT: String, QUERY: String): Boolean
  {
    if (QUERY.isBlank()) return true
    val cleanText  : String = TEXT.lowercase()
    val cleanQuery : String = QUERY.lowercase().trim()

    // - - - Simple fuzzy: check if all characters of query appear in text in order
    var textIndex  : Int = 0
    var queryIndex : Int = 0
    while (textIndex < cleanText.length && queryIndex < cleanQuery.length)
    {
      if (cleanText[textIndex] == cleanQuery[queryIndex]) { queryIndex++ }
      textIndex++
    }
    return queryIndex == cleanQuery.length
  }

  /** Modifies the active text filtering constraint backing query execution pipelines. */
  fun updateSearchQuery(QUERY : String)
  { _searchQuery.value = QUERY }

  /** Inverts relational visibility flags on target rows and posts updates onto persistent storage. */
  fun toggleFavorite(GAME : Game)
  {
    viewModelScope.launch { REPO.updateGame(GAME.copy(isFavorite = !GAME.isFavorite)) }
  }

  /** Commits direct data modifications adding graphic path link fields into targeted database profiles. */
  fun updateBoxArtUrl(GAME: Game, URL: String)
  {
    viewModelScope.launch ()
    {
      REPO.updateGame(GAME.copy(boxArtUrl = URL))
      _selectedGame.emit(null)
    }
  }

  /** Registers ongoing system access timestamps on disk before routing navigation to host views. */
  fun markAsPlayed(GAME : Game)
  {
    viewModelScope.launch ()
    {
      REPO.updateLastPlayed(GAME.id, System.currentTimeMillis())
      App.appModule.navigator.navigate(Destination.Emulator(GAME.romUri))
    }
  }

  /** Triggers directory file tracking sweeps to synchronize localized app catalog sheets with system folders. */
  fun detectAndInsertRoms()
  {
    viewModelScope.launch ()
    {
      val key                     = "GAME_BOY_ROMS"
      val repo                    = App.appModule.repo
      repo.syncGamesWithStorage(key)
    }
  }

  /** Posts structural focused data configurations onto active UI evaluation observers. */
  fun selectGame(GAME : Game?)
  {
    viewModelScope.launch { _selectedGame.emit(GAME) }
  }

  private val boxArtFetcher = App.appModule.boxArtFetcher

  private val _boxArtMap  : MutableStateFlow<Map<String, String?>> = MutableStateFlow(emptyMap())
  val boxArtMap           : StateFlow<Map<String, String?>> = _boxArtMap

  private val boxArtQueue  = Channel<String>(capacity = Channel.UNLIMITED)
  private val queuedTitles = Collections.synchronizedSet(mutableSetOf<String>())

  /**
   * Checks mapping states to emit paths to promotional illustrations matching local keys.
   *
   * @param game Targeted context structure query reference object.
   * @return Safe flow container broadcasting structural asset URL links.
   */
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
    // Deprecated
  }

  private fun fetchBoxArt(GAME: Game)
  {
    val title = GAME.title
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
              // - - - Cache to DB if found
              REPO.getGameByTitle(title)?.let()
              { game ->
                if (game.boxArtUrl == null)
                { REPO.updateGame(game.copy(boxArtUrl = url)) }
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