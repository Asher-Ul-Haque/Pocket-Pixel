package just.somebody.templates.domain.repositories

import just.somebody.templates.domain.models.Game
import kotlinx.coroutines.flow.Flow

interface GameRepository
{
  suspend fun insertGame (GAME  : Game)
  suspend fun insertGames( KEY  : String)
  suspend fun updateGame (GAME  : Game)
  suspend fun deleteGame (GAME  : Game)

          fun getAllGames    () : Flow<List<Game>>
  suspend fun getAllGamesOnce() : List<Game>

  suspend fun getGameById    (ID    : Long)   : Game?
  suspend fun getGameByTitle (TITLE : String) : Game?
  suspend fun getGameByUri   (URI   : String) : Game?

          fun getFavoriteGames    ()          : Flow<List<Game>>
          fun getFavoriteGamesOnce()          : List<Game>
  suspend fun isGameFavorite      (ID : Long) : Boolean

          fun getRecentlyPlayedGames()  : Flow<List<Game>>
          fun getNeverPlayedGames   ()  : Flow<List<Game>>
  suspend fun updateLastPlayed      (ID : Long, TIMESTAMP : Long)

  fun searchGames(QUERY : String) : Flow<List<Game>>
}