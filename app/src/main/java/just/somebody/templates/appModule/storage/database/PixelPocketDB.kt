package just.somebody.templates.appModule.storage.database

import androidx.room.Database
import androidx.room.RoomDatabase
import just.somebody.templates.data.daos.GameDao
import just.somebody.templates.data.entities.GameEntity

@Database(
  entities = [ GameEntity::class ],
  version  = 1
)
abstract class PixelPocketDB : RoomDatabase()
{
  abstract fun gameDAO() : GameDao
  companion object { val DB_NAME : String = "pixel_db"}
}