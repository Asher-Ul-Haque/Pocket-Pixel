package just.somebody.templates.domain.repositories

import just.somebody.templates.data.daos.CollectionDao
import just.somebody.templates.data.entities.CollectionEntity
import just.somebody.templates.data.entities.CollectionGameCrossRef
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.models.GameCollection
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

/**
 * Concrete implementation of [CollectionRepository] using Room DAOs.
 */
class DefaultCollectionRepository(private val DAO: CollectionDao) : CollectionRepository
{
  /** converts CollectionEntity to a Game Collection*/
  private fun CollectionEntity.toDomain(GAMES: List<Game> = emptyList()) = GameCollection(
    id       = id,
    name     = name,
    isSystem = isSystem,
    games    = GAMES)

  /** converts a GameEntity to a Game */
  private fun GameEntity.toDomain() = Game(
    id         = id,
    title      = title,
    romUri     = romUri,
    lastPlayed = lastPlayed,
    playTime   = playTime,
    isFavorite = isFavorite,
    boxArtUrl  = boxArtUrl)

  /** Returns a list of all the GameCollections */
  override fun getAllCollections(): Flow<List<GameCollection>> =
    DAO.getCollectionsWithGames().map()
    { list ->
      list.map()
      { it.collection.toDomain(it.games.map { g -> g.toDomain() }) }
    }

  /**
   * Creates a collection
   * @param NAME (String) : the name of the collection
   * @param IS_SYSTEM (Boolean) : whether the collection is created by the app or by the user
   * @return (Long) : the id of the new collection
   */
  override suspend fun createCollection(NAME: String, IS_SYSTEM: Boolean): Long =
    DAO.insertCollection(CollectionEntity(name = NAME, isSystem = IS_SYSTEM))

  /**
   * Updates collection
   * @param COLLECTION (GameCollection) : the collection to be updated
   */
  override suspend fun updateCollection(COLLECTION: GameCollection) =
    DAO.updateCollection(CollectionEntity(id = COLLECTION.id, name = COLLECTION.name, isSystem = COLLECTION.isSystem))

  /**
   * Deletes collection
   * @param COLLECTION (GameCollection) : the collection to be deleted
   */
  override suspend fun deleteCollection(COLLECTION: GameCollection) =
    DAO.deleteCollection(CollectionEntity(id = COLLECTION.id, name = COLLECTION.name, isSystem = COLLECTION.isSystem))

  /**
   * Adds a game to the collection
   * @param COLLECTION_ID (Long) : the id of the collection
   * @param GAME_ID (Long) : the id of the game to be added
   */
  override suspend fun addGameToCollection(COLLECTION_ID: Long, GAME_ID: Long) =
    DAO.addGameToCollection(CollectionGameCrossRef(COLLECTION_ID, GAME_ID))

  /**
   * Removes a game from a collection
   * @param COLLECTION_ID (Long) : the id of the collection
   * @param GAME_ID (Long) : the id of the game to be added
   */
  override suspend fun removeGameFromCollection(COLLECTION_ID: Long, GAME_ID: Long) =
    DAO.removeGameFromCollection(CollectionGameCrossRef(COLLECTION_ID, GAME_ID))

  /**
   * Returns a collection with any games
   * @param COLLECTION_ID (Long) : the collection id
   * return GameCollection? : the collection if it has any games
   */
  override fun getCollectionWithGames(COLLECTION_ID: Long): Flow<GameCollection?> =
    DAO.getCollectionWithGames(COLLECTION_ID).map()
    { rel ->
      rel?.let { it.collection.toDomain(it.games.map { g -> g.toDomain() }) }
    }

  /** Makes sure that the system collections are created and updated */
  override suspend fun ensureSystemCollections()
  {
    val collections = DAO.getCollectionById(1) // - - - Assuming 1 is Favorites
    if (collections == null)
    {
      DAO.insertCollection(CollectionEntity(id = 1, name = "Favorites", isSystem = true))
    }
  }
}