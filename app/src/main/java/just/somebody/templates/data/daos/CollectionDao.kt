package just.somebody.templates.data.daos

import androidx.room.*
import just.somebody.templates.data.entities.CollectionEntity
import just.somebody.templates.data.entities.CollectionGameCrossRef
import just.somebody.templates.data.entities.GameEntity
import kotlinx.coroutines.flow.Flow

/**
 * Interface mapping data access operations for user-defined and system game collections.
 */
@Dao
interface CollectionDao
{
  @Insert(onConflict = OnConflictStrategy.REPLACE)
  suspend fun insertCollection(collection: CollectionEntity): Long

  @Update
  suspend fun updateCollection(collection: CollectionEntity)

  @Delete
  suspend fun deleteCollection(collection: CollectionEntity)

  @Query("SELECT * FROM collections")
  fun getAllCollections(): Flow<List<CollectionEntity>>

  @Query("SELECT * FROM collections WHERE id = :id")
  suspend fun getCollectionById(id: Long): CollectionEntity?

  @Insert(onConflict = OnConflictStrategy.IGNORE)
  suspend fun addGameToCollection(crossRef: CollectionGameCrossRef)

  @Delete
  suspend fun removeGameFromCollection(crossRef: CollectionGameCrossRef)

  @Transaction
  @Query("SELECT * FROM collections WHERE id = :collectionId")
  fun getCollectionWithGames(collectionId: Long): Flow<CollectionWithGames?>

  @Transaction
  @Query("SELECT * FROM collections")
  fun getCollectionsWithGames(): Flow<List<CollectionWithGames>>
}

data class CollectionWithGames(
  @Embedded val collection: CollectionEntity,
  @Relation(
    parentColumn = "id",
    entityColumn = "id",
    associateBy = Junction(
      CollectionGameCrossRef::class,
      parentColumn = "collectionId",
      entityColumn = "gameId"
    )
  )
  val games: List<GameEntity>
)
