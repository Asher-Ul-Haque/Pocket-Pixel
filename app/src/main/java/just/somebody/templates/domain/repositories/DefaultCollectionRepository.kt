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
class DefaultCollectionRepository(private val dao: CollectionDao) : CollectionRepository
{
  private fun CollectionEntity.toDomain(games: List<Game> = emptyList()) = GameCollection(
    id       = id,
    name     = name,
    isSystem = isSystem,
    games    = games
  )

  private fun GameEntity.toDomain() = Game(
    id         = id,
    title      = title,
    romUri     = romUri,
    lastPlayed = lastPlayed,
    playTime   = playTime,
    isFavorite = isFavorite,
    boxArtUrl  = boxArtUrl
  )

  override fun getAllCollections(): Flow<List<GameCollection>> =
    dao.getAllCollections().map { list -> list.map { it.toDomain() } }

  override suspend fun createCollection(name: String, isSystem: Boolean): Long =
    dao.insertCollection(CollectionEntity(name = name, isSystem = isSystem))

  override suspend fun updateCollection(collection: GameCollection) =
    dao.updateCollection(CollectionEntity(id = collection.id, name = collection.name, isSystem = collection.isSystem))

  override suspend fun deleteCollection(collection: GameCollection) =
    dao.deleteCollection(CollectionEntity(id = collection.id, name = collection.name, isSystem = collection.isSystem))

  override suspend fun addGameToCollection(collectionId: Long, gameId: Long) =
    dao.addGameToCollection(CollectionGameCrossRef(collectionId, gameId))

  override suspend fun removeGameFromCollection(collectionId: Long, gameId: Long) =
    dao.removeGameFromCollection(CollectionGameCrossRef(collectionId, gameId))

  override fun getCollectionWithGames(collectionId: Long): Flow<GameCollection?> =
    dao.getCollectionWithGames(collectionId).map { rel ->
      rel?.let { it.collection.toDomain(it.games.map { g -> g.toDomain() }) }
    }
}
