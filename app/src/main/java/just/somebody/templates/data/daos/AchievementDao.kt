package just.somebody.templates.data.daos

import androidx.room.*
import just.somebody.templates.data.entities.AchievementEntity
import kotlinx.coroutines.flow.Flow

@Dao
interface AchievementDao {
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertAchievement(ACHIEVEMENT: AchievementEntity)

    @Query("SELECT * FROM achievements WHERE gameId = :GAME_ID ORDER BY unlockDate DESC")
    fun getAchievementsForGame(GAME_ID: Long): Flow<List<AchievementEntity>>

    @Query("SELECT * FROM achievements ORDER BY unlockDate DESC")
    fun getAllAchievements(): Flow<List<AchievementEntity>>

    @Query("DELETE FROM achievements")
    suspend fun deleteAllAchievements()
}
