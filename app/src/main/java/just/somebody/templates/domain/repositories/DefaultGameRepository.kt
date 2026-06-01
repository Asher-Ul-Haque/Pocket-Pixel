package just.somebody.templates.domain.repositories

import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.domain.models.Game
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

/**
 * Concrete domain broker data controller managing mapping bridges over database data access operations.
 * Coordinates Storage Access Framework query steps to translate raw storage contents into core data properties.
 *
 * @property DAO Local access database driver mapping operations for persistent entries.
 */
class DefaultGameRepository(private val DAO : GameDao) : GameRepository
{
  /** Extension mapping method translating a relational [GameEntity] to a pure domain [Game] record. */
  private fun GameEntity.toDomain() : Game = Game(
    id              = id,
    title           = title,
    romUri          = romUri,
    lastPlayed      = lastPlayed,
    isFavorite      = isFavorite,
    boxArtUrl       = boxArtUrl)

  /** Extension mapping method translating a pure domain [Game] record to a relational [GameEntity]. */
  private fun Game.toEntity() : GameEntity = GameEntity(
    id              = id,
    title           = title,
    romUri          = romUri,
    lastPlayed      = lastPlayed,
    isFavorite      = isFavorite,
    boxArtUrl       = boxArtUrl)

  override suspend fun insertGames(KEY: String)
  {
    val storeManager  = App.appModule.externalStorageManager
    val docFiles      = storeManager
      .listFiles(KEY)
      .filter ()
      { file ->
        val name = file.name ?: return@filter false
        name.endsWith(".gb", ignoreCase = true) || name.endsWith(".gbc", ignoreCase = true)
      }

    val games = docFiles.mapNotNull()
    { file ->
      val name = file.name ?: return@mapNotNull null
      ForgeLogger.trace("Detected file : $name")
      val cleanName = name.replace(Regex("\\.gbc?$", RegexOption.IGNORE_CASE), "")
      Game(
        title      = cleanName,
        romUri     = file.uri.toString(),
        lastPlayed = null,
        isFavorite = false)
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

  override suspend fun syncGamesWithStorage(KEY : String)
  {
    val storeManager = App.appModule.externalStorageManager
    val directory    = storeManager.getDirectory(KEY)

    if (directory == null)
    {
      factoryReset()
      ForgeLogger.warn("Directory with key '$KEY' is missing or invalid, factory resetting")
      return
    }

    // - - - Step 1: Scan external storage for all .gb files
    val gbFiles  = storeManager.listFiles(KEY, EXTENSION = "gb", RECURSIVE = true)
    val gbcFiles = storeManager.listFiles(KEY, EXTENSION = "gbc", RECURSIVE = true)
    val docFiles = gbFiles + gbcFiles

    // - - - Step 2: Convert to Game domain objects
    val scannedGames = docFiles.mapNotNull()
    { file ->
      val name = file.name ?: return@mapNotNull null
      val uri  = file.uri.toString()

      Game(
        title           = name.replace(Regex("\\.gbc?$", RegexOption.IGNORE_CASE), ""),
        romUri          = uri,
        lastPlayed      = null,
        isFavorite      = false)
    }

    // - - - Step 3: Fetch games from database
    val dbGames     = getAllGamesOnce()
    val dbUris      = dbGames.map { it.romUri }.toSet()
    val scannedUris = scannedGames.map { it.romUri }.toSet()

    // - - - Step 4: Find and insert new games
    val newGames = scannedGames.filter { it.romUri !in dbUris }
    if (newGames.isNotEmpty())
    {
      ForgeLogger.info("Inserting ${newGames.size} new game(s).")
      DAO.insertGames(newGames.map { it.toEntity() })
    }

    // - - - Step 5: Find and delete missing games
    val missingGames = dbGames.filter { it.romUri !in scannedUris }
    if (missingGames.isNotEmpty())
    {
      ForgeLogger.info("Deleting ${missingGames.size} removed game(s).")
      DAO.deleteGames(missingGames.map { it.toEntity() })
    }

    ForgeLogger.info("Sync complete: ${newGames.size} added, ${missingGames.size} removed.")
  }
}