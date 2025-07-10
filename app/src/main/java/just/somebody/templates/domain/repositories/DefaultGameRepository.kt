package just.somebody.templates.domain.repositories

import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.domain.models.Game
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

class DefaultGameRepository(private val DAO : GameDao) : GameRepository
{
  private fun GameEntity.toDomain() : Game = Game(
    id              = id,
    title           = title,
    publisher       = publisher,
    romUri          = romUri,
    batterySavePath = batterySave,
    lastPlayed      = lastPlayed,
    isFavorite      = isFavorite
  )

  private fun Game.toEntity() : GameEntity = GameEntity(
    id              = id,
    title           = title,
    publisher       = publisher,
    romUri          = romUri,
    batterySave     = batterySavePath,
    lastPlayed      = lastPlayed,
    isFavorite      = isFavorite
  )

  override suspend fun insertGames(KEY : String)
  {
    val storeManager = App.appModule.externalStorageManager
    val docFiles =
      storeManager.listFiles(KEY)
        .filter { file -> file.name?.endsWith(".gb", true) == true}
    val games = docFiles.mapNotNull()
    { file ->
      val name = file.name ?: return@mapNotNull null
      ForgeLogger.trace("Detected file : $name")
      Game(
        title           = name.removeSuffix(".gb"),
        publisher       = "Unkown",
        romUri          = file.uri.toString(),
        batterySavePath = null,
        lastPlayed      = null,
        isFavorite      = false
      )
    }
    DAO.insertGames(games.map { it.toEntity() })
  }

  override suspend fun insertGame(GAME : Game)
  { DAO.insertGame(GAME.toEntity()) }

  override suspend fun updateGame(GAME : Game)
  { DAO.updateGame(GAME.toEntity()) }

  override suspend fun deleteGame(GAME : Game)
  { DAO.deleteGame(GAME.toEntity()) }

  override fun getAllGames(): Flow<List<Game>> =
    DAO.getAllGames().map { it.map { it.toDomain() } }

  override suspend fun getAllGamesOnce(): List<Game> =
    DAO.getAllGamesOnce().map { it.toDomain() }

  override suspend fun getGameById(ID : Long): Game? =
    DAO.getGameByID(ID)?.toDomain()

  override suspend fun getGameByTitle(TITLE : String) : Game? =
    DAO.getGameByTitle(TITLE)?.toDomain()

  override suspend fun getGameByUri(URI : String): Game? =
    DAO.getGameByUri(URI)?.toDomain()

  override fun getFavoriteGames(): Flow<List<Game>> =
    DAO.getFavoriteGames().map { it.map { it.toDomain() } }

  override fun getFavoriteGamesOnce(): List<Game> =
    DAO.getFavoriteGamesOnce().map { it.toDomain() }

  override suspend fun isGameFavorite(ID : Long): Boolean =
    DAO.isGameFavorite(ID)

  override fun getRecentlyPlayedGames(): Flow<List<Game>> =
    DAO.getRecentlyPlayedGames().map { it.map { it.toDomain() } }

  override fun getNeverPlayedGames(): Flow<List<Game>> =
    DAO.getNeverPlayedGames().map { it.map { it.toDomain() } }

  override suspend fun updateLastPlayed(ID : Long, TIMESTAMP: Long)
  { DAO.updateLastPlayed(ID, TIMESTAMP) }

  override fun searchGames(QUERY : String): Flow<List<Game>> =
    DAO.searchGames(QUERY).map { it.map { it.toDomain() } }

  override suspend fun factoryReset()
  { DAO.deleteEverything() }
}
