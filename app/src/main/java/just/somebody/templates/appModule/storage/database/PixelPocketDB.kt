package just.somebody.templates.appModule.storage.database

import androidx.room.Database
import androidx.room.RoomDatabase
import just.somebody.templates.data.daos.AchievementDao
import just.somebody.templates.data.daos.CollectionDao
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.daos.SaveStateDao
import just.somebody.templates.data.entities.AchievementEntity
import just.somebody.templates.data.entities.CollectionEntity
import just.somebody.templates.data.entities.CollectionGameCrossRef
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.data.entities.SaveStateEntity

/**
 * The primary localized Room database layer managing structures for system entities and relational records.
 *
 * Maintains absolute synchronization over core emulation items including catalogued games
 * and volatile visual save configurations.
 */
@Database(
  entities = [
    GameEntity::class,
    SaveStateEntity::class,
    CollectionEntity::class,
    CollectionGameCrossRef::class,
    AchievementEntity::class ],
  version  = 9)
abstract class PixelPocketDB : RoomDatabase()
{
  /**
   * Exposes operational access capabilities targeting the local tracking inventory database records.
   *
   * @return Implementation layer mapping data access methods for game entries.
   */
  abstract fun gameDAO() : GameDao

  /**
   * Exposes operational access capabilities targeting transaction steps for saved simulation profiles.
   *
   * @return Implementation layer mapping data access methods for serialization captures.
   */
  abstract fun saveStateDAO() : SaveStateDao

  /**
   * Exposes operational access capabilities targeting transaction steps for game collections.
   *
   * @return Implementation layer mapping data access methods for collections.
   */
  abstract fun collectionDAO() : CollectionDao

  /**
   * Exposes operational access capabilities targeting transaction steps for achievements.
   *
   * @return Implementation layer mapping data access methods for achievements.
   */
  abstract fun achievementDAO() : AchievementDao

  companion object
  { /** Name tag defining the tracking workspace file instance allocated inside the OS path. */
    val DB_NAME : String = "pixel_db"
  }
}