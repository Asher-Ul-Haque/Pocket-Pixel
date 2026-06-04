package just.somebody.templates.domain.repositories

import just.somebody.templates.domain.models.Game
import kotlinx.coroutines.flow.Flow

/**
 * Domain-layer abstraction boundary defining database data transfer operations for the application.
 *
 * Exposes clean, architecture-agnostic queries, data streaming tracks, storage tracking logic,
 * and full platform-to-database index synchronization hooks.
 */
interface GameRepository
{
  /** Encodes and registers a unique game item metadata context into storage rows. */
  suspend fun insertGame (GAME  : Game)

  /** Scrapes, extracts, and registers structural files matching a storage profile key location. */
  suspend fun insertGames( KEY  : String)

  /** Commits modifications across mutable properties of an existing catalog entry. */
  suspend fun updateGame (GAME  : Game)

  /** Unlinks and deletes a matching record entry item from internal data tables. */
  suspend fun deleteGame (GAME  : Game)

  /** Establishes a cold data stream observing all catalogued data items inside the database indices. */
  fun getAllGames    () : Flow<List<Game>>

  /** Compiles a point-in-time array snapshot listing every registered game item block. */
  suspend fun getAllGamesOnce() : List<Game>

  /** Resolves and returns an entry row matching a specific unique primary key target [ID]. */
  suspend fun getGameById    (ID    : Long)   : Game?

  /** Identifies and yields records matching an exact case alphanumeric query phrase parameter. */
  suspend fun getGameByTitle (TITLE : String) : Game?

  /** Resolves file configurations locating rows targeted on matching platform storage tree strings. */
  suspend fun getGameByUri   (URI   : String) : Game?

  /** Opens a continuous stream pipeline filtering elements carrying explicit favorited state markers. */
  fun getFavoriteGames    ()          : Flow<List<Game>>

  /** Synthesizes an array collection tracking all elements currently marked as explicit favorites. */
  fun getFavoriteGamesOnce()          : List<Game>

  /** Inspects entry properties to check the binary validation flag for a target row reference. */
  suspend fun isGameFavorite      (ID : Long) : Boolean

  /** Tracks usage histories, sorting returning payloads descending based on access time ticks. */
  fun getRecentlyPlayedGames()  : Flow<List<Game>>

  /** Compiles a collection tracking registered assets that have never been actively initialized. */
  fun getNeverPlayedGames   ()  : Flow<List<Game>>

  /** Registers and writes updated system clock metadata values onto a target entry key cell. */
  suspend fun updateLastPlayed      (ID : Long, TIMESTAMP : Long)

  /** Increments the total accumulation of time spent actively running a specific game engine profile. */
  suspend fun updatePlayTime        (ID : Long, INCREMENT : Long)

  /** Ranks and retrieves the top most engaged game entries based on total accumulated play time metrics. */
  fun getTopMostPlayedGames(LIMIT : Int = 10) : Flow<List<Game>>

  /** Executes localized partial phrase string match sorting lookups against data indices. */
  fun searchGames(QUERY : String) : Flow<List<Game>>

  /**
   * Evaluates directory trees to align internal databases with physical files.
   *
   * Automatically drops unlinked storage nodes while initializing newly detected entries.
   *
   * @param KEY Platform directory registration keyword used to query files.
   */
  suspend fun syncGamesWithStorage(KEY : String)

  /** Truncates, drops, and purges the internal local index records entirely. */
  suspend fun factoryReset()
}