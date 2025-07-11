package just.somebody.templates.data.daos

import androidx.room.Dao
import androidx.room.Delete
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import androidx.room.Update
import just.somebody.templates.data.entities.GameEntity
import kotlinx.coroutines.flow.Flow

@Dao
interface GameDao
{
  // - - - Insert Update Delete - - -
  @Insert(onConflict = OnConflictStrategy.IGNORE)
  suspend fun insertGame(GAME   : GameEntity)

  @Insert(onConflict = OnConflictStrategy.IGNORE)
  suspend fun insertGames(GAMES : List<GameEntity>)

  @Update suspend fun updateGame(GAME : GameEntity)

  @Delete suspend fun deleteGame(GAME : GameEntity)


  // - - - Getters - - -
  @Query("SELECT * FROM games")
  fun getAllGames() : Flow<List<GameEntity>>

  @Query("SELECT * FROM games")
  suspend fun getAllGamesOnce() : List<GameEntity>

  @Query("SELECT * FROM games WHERE id = :ID LIMIT 1")
  suspend fun getGameByID(ID : Long) : GameEntity?

  @Query("SELECT * FROM games WHERE title = :TITLE LIMIT 1")
  suspend fun getGameByTitle(TITLE : String) : GameEntity?

  @Query("SELECT * FROM games WHERE romUri = :URI LIMIT 1")
  suspend fun getGameByUri(URI : String) : GameEntity?


  // - - - Favorites - - -
  @Query("SELECT * FROM games WHERE isFavorite = 1")
  fun getFavoriteGames() : Flow<List<GameEntity>>

  @Query("SELECT * FROM games WHERE isFavorite = 1")
  fun getFavoriteGamesOnce() : List<GameEntity>

  @Query("SELECT isFavorite FROM games WHERE id = :ID")
  suspend fun isGameFavorite(ID : Long) : Boolean


  // - - - Last played - - -
  @Query("SELECT * FROM games WHERE lastPlayed IS NOT NULL ORDER BY lastPlayed DESC")
  fun getRecentlyPlayedGames() : Flow<List<GameEntity>>

  @Query("SELECT * FROM games WHERE lastPlayed IS NULL")
  fun getNeverPlayedGames() : Flow<List<GameEntity>>

  @Query("UPDATE games SET lastPlayed = :TIMESTAMP WHERE id = :ID")
  suspend fun updateLastPlayed(ID : Long, TIMESTAMP : Long)


  // - - - Search fuzzy - - -
  @Query("""
        SELECT * FROM games 
        WHERE title LIKE '%' || :QUERY || '%' OR publisher LIKE '%' || :QUERY || '%'
        ORDER BY 
           CASE WHEN lastPlayed IS NULL THEN 1 ELSE 0 END,
           lastPlayed DESC""")
  fun searchGames(QUERY : String): Flow<List<GameEntity>>


  // - - - Drop all - - -
  @Query("DELETE FROM games")
  suspend fun deleteEverything()
}