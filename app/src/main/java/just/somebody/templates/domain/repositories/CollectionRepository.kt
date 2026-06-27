package just.somebody.templates.domain.repositories

import just.somebody.templates.domain.models.GameCollection
import kotlinx.coroutines.flow.Flow

/**
 * Domain-layer abstraction for managing game collections.
 */
interface CollectionRepository
{
  /**
   * Returns a list of all collections
   * @return the list of all collections
   */
  fun getAllCollections(): Flow<List<GameCollection>>

  /**
   * Creates a new collection
   * @param NAME (String): The name of the collection
   * @param IS_SYSTEM (Boolean): is the collection created by the app or by the user
   * @return (Long) the id of the collection
   */
  suspend fun createCollection(NAME: String, IS_SYSTEM: Boolean = false): Long

  /**
   * Updates a collection
   * @param COLLECTION (GameCollection) : The collection to be updated
   */
  suspend fun updateCollection(COLLECTION: GameCollection)

  /**
   * Deletes a collection
   * @param COLLECTION (GameCollection) : the collection to be destroyed
   */
  suspend fun deleteCollection(COLLECTION: GameCollection)

  /**
   * Add a game to a collection
   * @param COLLECTION_ID (Long) : the id of the collection to which the game is to be added
   * @param GAME_ID (Long) : the id of the game to be added
   */
  suspend fun addGameToCollection(COLLECTION_ID: Long, GAME_ID: Long)

  /**
   * Remove a game from a collection
   * @param COLLECTION_ID (Long) : the id of the collection from which the game is to be removed
   * @param GAME_ID (Long) : the id of the game to be removed
   */
  suspend fun removeGameFromCollection(COLLECTION_ID: Long, GAME_ID: Long)

  /**
   * Returns a game collection with any games in it
   * @param COLLECTION_ID (Long) : the id of the collection
   * @return (GameCollection?) : the game collection if it has any games
   */
  fun getCollectionWithGames(COLLECTION_ID: Long): Flow<GameCollection?>

  /**
   * Makes sure the system collections are made and updated
   */
  suspend fun ensureSystemCollections()
}
