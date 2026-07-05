package just.somebody.templates.data.daos

import androidx.room.*
import just.somebody.templates.data.entities.AchievementEntity
import kotlinx.coroutines.flow.Flow

@Dao
interface AchievementDao {
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertAchievement(ACHIEVEMENT: AchievementEntity)

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertAchievements(ACHIEVEMENTS: List<AchievementEntity>)

    @Query("SELECT * FROM achievements WHERE raGameId = :RA_GAME_ID ORDER BY raId ASC")
    fun getAchievementsForRaGame(RA_GAME_ID: Int): Flow<List<AchievementEntity>>

    @Query("SELECT * FROM achievements WHERE gameId = :GAME_ID ORDER BY unlockDate DESC")
    fun getAchievementsForGame(GAME_ID: Long): Flow<List<AchievementEntity>>

    @Query("SELECT * FROM achievements WHERE gameId = :GAME_ID ORDER BY unlockDate DESC")
    suspend fun getAchievementsForGameOnce(GAME_ID: Long): List<AchievementEntity>

    @Query("SELECT * FROM achievements ORDER BY unlockDate DESC")
    fun getAllAchievements(): Flow<List<AchievementEntity>>

    @Query("SELECT * FROM achievements WHERE raId = :RA_ID LIMIT 1")
    suspend fun getAchievementById(RA_ID: Int): AchievementEntity?

    @Query("DELETE FROM achievements WHERE gameId = :GAME_ID")
    suspend fun deleteAllAchievementsByGameId(GAME_ID: Long)

    @Query("DELETE FROM achievements")
    suspend fun deleteAllAchievements()
}
