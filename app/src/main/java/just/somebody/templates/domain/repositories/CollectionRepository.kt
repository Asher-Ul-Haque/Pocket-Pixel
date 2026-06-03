package just.somebody.templates.domain.repositories

import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.models.GameCollection
import kotlinx.coroutines.flow.Flow

/**
 * Domain-layer abstraction for managing game collections.
 */
interface CollectionRepository
{
  fun getAllCollections(): Flow<List<GameCollection>>
  
  suspend fun createCollection(name: String, isSystem: Boolean = false): Long
  
  suspend fun updateCollection(collection: GameCollection)
  
  suspend fun deleteCollection(collection: GameCollection)
  
  suspend fun addGameToCollection(collectionId: Long, gameId: Long)
  
  suspend fun removeGameFromCollection(collectionId: Long, gameId: Long)
  
  fun getCollectionWithGames(collectionId: Long): Flow<GameCollection?>
}
