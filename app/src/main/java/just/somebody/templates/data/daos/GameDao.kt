package just.somebody.templates.data.daos

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Update
import just.somebody.templates.data.entities.GameEntity
import kotlinx.coroutines.flow.Flow

/**
 * Interface mapping data access operations targeting the local persistent structural games index table.
 * Resolves metadata tracking metrics, favorite settings state markers, usage histories, and pattern queries.
 */
@Dao
interface GameDao
{
  // - - - Insert Update Delete - - -
  /** Registers a unique record entry to the catalog, ignoring conflicts if duplicates are registered. */
  @Insert(onConflict = OnConflictStrategy.IGNORE)
  suspend fun insertGame(GAME   : GameEntity)

  /** Dispatches batch array lists onto persistent storage while avoiding duplicate entry overlaps. */
  @Insert(onConflict = OnConflictStrategy.IGNORE)
  suspend fun insertGames(GAMES : List<GameEntity>)

  /** Commits modified data fields across an existing structured matrix entity record. */
  @Update suspend fun updateGame(GAME : GameEntity)

  /** Unlinks and deletes a matching structured game row entry item from persistent tables. */
  @Delete suspend fun deleteGame(GAME : GameEntity)

  /** Executes batch deletions against a collection matrix listing targeting active tracking layers. */
  @Delete suspend fun deleteGames(GAMES : List<GameEntity>)


  // - - - Getters - - -
  /** Compiles a continuous reactive stream observation tracking all catalogued entities inside storage. */
  @Query("SELECT * FROM games")
  fun getAllGames() : Flow<List<GameEntity>>

  /** Issues a singular snapshot fetch extracting the full array matrix tracking saved game titles. */
  @Query("SELECT * FROM games")
  suspend fun getAllGamesOnce() : List<GameEntity>

  /** Searches index matrices to isolate a matching item record row pointing to a distinct unique target [ID]. */
  @Query("SELECT * FROM games WHERE id = :ID LIMIT 1")
  suspend fun getGameByID(ID : Long) : GameEntity?

  /** Identifies and returns metadata structures matching an exact case alphanumeric lookup string parameter. */
  @Query("SELECT * FROM games WHERE title = :TITLE LIMIT 1")
  suspend fun getGameByTitle(TITLE : String) : GameEntity?

  /** Resolves asset configuration rows locating matches targeted on platform Storage Access Framework paths. */
  @Query("SELECT * FROM games WHERE romUri = :URI LIMIT 1")
  suspend fun getGameByUri(URI : String) : GameEntity?


  // - - - Favorites - - -
  /** Establishes a cold data observation pipeline filtering records carrying explicit positive priority tags. */
  @Query("SELECT * FROM games WHERE isFavorite = 1")
  fun getFavoriteGames() : Flow<List<GameEntity>>

  /** Synthesizes a point-in-time array vector compiling all items flagged as explicit favorites. */
  @Query("SELECT * FROM games WHERE isFavorite = 1")
  fun getFavoriteGamesOnce() : List<GameEntity>

  /** Checks tracking properties to extract a singular toggle confirmation value state tracking row targets. */
  @Query("SELECT isFavorite FROM games WHERE id = :ID")
  suspend fun isGameFavorite(ID : Long) : Boolean


  // - - - Last played - - -
  /** Tracks chronological usage matrices, returning items sorted descending by historical access time keys. */
  @Query("SELECT * FROM games WHERE lastPlayed IS NOT NULL ORDER BY lastPlayed DESC")
  fun getRecentlyPlayedGames() : Flow<List<GameEntity>>

  /** Compiles a structural listing filtering out catalogued entries that have never been actively initialized. */
  @Query("SELECT * FROM games WHERE lastPlayed IS NULL")
  fun getNeverPlayedGames() : Flow<List<GameEntity>>

  /** Registers and writes updated system clock metadata ticks onto a specific historical target row node. */
  @Query("UPDATE games SET lastPlayed = :TIMESTAMP WHERE id = :ID")
  suspend fun updateLastPlayed(ID : Long, TIMESTAMP : Long)


  // - - - Search fuzzy - - -
  /**
   * Executes partial string fuzzy pattern matching across storage blocks.
   *
   * Automatically organizes returning payloads to bubble up historically requested items
   * prioritized directly ahead of unplayed assets.
   *
   * @param QUERY Filter key segment used to inspect character arrays.
   * @return A stream container packing sorting order maps matching parameter bounds.
   */
  @Query("""
        SELECT * FROM games 
        WHERE title LIKE '%' || :QUERY || '%' 
        ORDER BY 
           CASE WHEN lastPlayed IS NULL THEN 1 ELSE 0 END,
           lastPlayed DESC""")
  fun searchGames(QUERY : String): Flow<List<GameEntity>>


  // - - - Drop all - - -
  /** Drops, clears, and truncates the global tracking layout database indices entirely. */
  @Query("DELETE FROM games")
  suspend fun deleteEverything()
}