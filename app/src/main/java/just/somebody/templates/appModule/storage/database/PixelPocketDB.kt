package just.somebody.templates.appModule.storage.database

import androidx.room.Database
import androidx.room.RoomDatabase
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.daos.SaveStateDao
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.data.entities.SaveStateEntity

/**
 * The primary localized Room database layer managing structures for system entities and relational records.
 *
 * Maintains absolute synchronization over core emulation items including catalogued games
 * and volatile visual save configurations.
 */
@Database(
  entities = [ GameEntity::class, SaveStateEntity::class ],
  version  = 6)
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

  companion object
  { /** Name tag defining the tracking workspace file instance allocated inside the OS path. */
    val DB_NAME : String = "pixel_db"
  }
}