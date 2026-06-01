package just.somebody.templates.data.daos

import androidx.room.*
import just.somebody.templates.data.entities.SaveStateEntity
import kotlinx.coroutines.flow.Flow

/**
 * Interface mapping data access operations targeting the serialization cache tables.
 * Maintains historical hardware register snapshots and companion indexing profiles.
 */
@Dao
interface SaveStateDao
{
  /**
   * Extracts a specific core snapshot record instance matching explicit composite search criteria coordinates.
   *
   * @param gameId Parent database tracking key assigned to the item record target.
   * @param slot The distinct allocation slot sector indexing the historical slice.
   * @return Managed state metadata block or null if reference pointer location contains no data.
   */
  @Query("SELECT * FROM save_states WHERE gameId = :GAME_ID AND slot = :SLOT LIMIT 1")
  suspend fun getSaveState(GAME_ID: Long, SLOT: Int): SaveStateEntity?

  /**
   * Continuous observational track monitoring structural allocations belonging to a common target application identifier.
   *
   * @param gameId Parent database tracking key assigned to the item record target.
   * @return Sorted asynchronous sequence listing all active data snapshot containers.
   */
  @Query("SELECT * FROM save_states WHERE gameId = :GAME_ID ORDER BY slot ASC")
  fun getSaveStatesForGame(GAME_ID: Long): Flow<List<SaveStateEntity>>

  /** Commits a memory backup context onto storage, overwriting existing layouts if matching keys clash. */
  @Insert(onConflict = OnConflictStrategy.REPLACE)
  suspend fun insertSaveState(SAVE_STATE: SaveStateEntity)

  /** Truncates and deletes an explicit memory state record mapping segment out from persistent storage. */
  @Delete
  suspend fun deleteSaveState(SAVE_STATE: SaveStateEntity)

  /** Clears and purges every transactional snapshot mapped matching a parent structural identifier constraint. */
  @Query("DELETE FROM save_states WHERE gameId = :GAME_ID")
  suspend fun deleteAllSaveStatesForGame(GAME_ID: Long)
}