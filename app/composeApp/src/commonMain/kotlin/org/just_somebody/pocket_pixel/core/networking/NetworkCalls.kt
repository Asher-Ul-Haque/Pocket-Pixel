package org.just_somebody.pocket_pixel.core.networking

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.request.*
import io.ktor.client.statement.*
import io.ktor.http.*
import org.just_somebody.pocket_pixel.core.DataError
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.core.Gamer
import org.just_somebody.pocket_pixel.core.Result

class NetworkCalls
{
  private val client  : HttpClient  = createHttpClient(getHttpEngine())
  private val baseUrl : String      = "parthKeFunde.com"

  suspend fun loginGamer(GAMER: Gamer) : Result<Boolean, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for login is : '/auth/login'
      2. if I pass gamerTag and passWord hash as parameters it ought to work
     */
    return safeCall ()
    {
      client.get("$baseUrl/auth/login")
      {
        parameter("gamerTag", GAMER.name)
        parameter("password", GAMER.password)
      }
    }
  }

  suspend fun registerGamer(GAMER: Gamer): Result<Boolean, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for register is : '/auth/register'
      2. if I pass gamerTag and passWord hash as parameters it ought to work
     */
    return safeCall ()
    {
      client.get("$baseUrl/auth/register")
      {
        parameter("gamerTag", GAMER.name)
        parameter("password", GAMER.password)
      }
    }
  }

  suspend fun getFavoriteGames(GAMER: Gamer): Result<List<Game>, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for favorites is : '/favorites'
      2. if I pass gamerTag as parameter, it ought to work
     */
    return safeCall () { client.get("$baseUrl/favorites") { parameter("gamerTag", GAMER.name) } }
  }

  suspend fun searchGames(SEARCH_TERM : String): Result<List<Game>, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for search is : '/search'
      2. if I pass only the searchTerm, I get a result
     */
    return safeCall () { client.get("$baseUrl/search") { parameter("searchTerm", SEARCH_TERM) } }
  }

  suspend fun getGameROM(GAME : Game) : Result<ByteArray, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for game binary is : '/game/rom'
      2. if I pass only the game, I get a result
   */
    return safeCall () { client.get("$baseUrl/game/rom") { parameter("gameTitle", GAME.title) } }
  }

  suspend fun getGameSave(GAME: Game, GAMER: Gamer) : Result<ByteArray, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for game saves is : '/game/saves/download'
      2. if I pass only the game, I get a result
    */
    return safeCall ()
    {
      client.get("$baseUrl/game/saves/download")
      {
        parameter("gameTitle", GAME.title)
        parameter("gamerTag",  GAMER.name)
      }
    }
  }

  suspend fun setGameSave(GAME: Game, GAMER: Gamer, BYTE_ARRAY : ByteArray) : Result<Boolean, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for game saves is : '/game/saves'
      2. if I pass only the game, I get a result
    */
    return safeCall ()
    {
      client.get("$baseUrl/game/saves/upload")
      {
        parameter("gameTitle", GAME.title)
        parameter("gamerTag",  GAMER.name)
        parameter("saveFile",  BYTE_ARRAY)
      }
    }
  }

  suspend fun likeGame(GAME: Game, GAMER: Gamer, LIKE : Boolean) : Result<ByteArray, DataError.Remote>
  {
    /* - - - WARNING: Assumptions:
      1. the end point for favorites is : '/favorites'
      2. if I pass only the game, I get a result
    */
    return safeCall ()
    {
      client.get("$baseUrl/game/favorites/like")
      {
        parameter("gameTitle",  GAME.title)
        parameter("gamerTag",   GAMER.name)
        parameter("like",       LIKE)
      }
    }
  }
}
