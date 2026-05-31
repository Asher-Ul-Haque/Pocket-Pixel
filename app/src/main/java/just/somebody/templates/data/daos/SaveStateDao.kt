package just.somebody.templates.data.daos

import androidx.room.*
import just.somebody.templates.data.entities.SaveStateEntity
import kotlinx.coroutines.flow.Flow

@Dao
interface SaveStateDao {
    @Query("SELECT * FROM save_states WHERE gameId = :gameId AND slot = :slot LIMIT 1")
    suspend fun getSaveState(gameId: Long, slot: Int): SaveStateEntity?

    @Query("SELECT * FROM save_states WHERE gameId = :gameId ORDER BY slot ASC")
    fun getSaveStatesForGame(gameId: Long): Flow<List<SaveStateEntity>>

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertSaveState(saveState: SaveStateEntity)

    @Delete
    suspend fun deleteSaveState(saveState: SaveStateEntity)

    @Query("DELETE FROM save_states WHERE gameId = :gameId")
    suspend fun deleteAllSaveStatesForGame(gameId: Long)
}
