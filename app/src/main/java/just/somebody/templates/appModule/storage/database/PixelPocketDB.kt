package just.somebody.templates.appModule.storage.database

import androidx.room.Database
import androidx.room.RoomDatabase
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.daos.SaveStateDao
import just.somebody.templates.data.entities.GameEntity
import just.somebody.templates.data.entities.SaveStateEntity

@Database(
  entities = [ GameEntity::class, SaveStateEntity::class ],
  version  = 5
)
abstract class PixelPocketDB : RoomDatabase()
{
  abstract fun gameDAO() : GameDao
  abstract fun saveStateDAO() : SaveStateDao
  companion object { val DB_NAME : String = "pixel_db"}
}