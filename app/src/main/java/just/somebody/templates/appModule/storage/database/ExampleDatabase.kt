package just.somebody.templates.appModule.storage.database

import androidx.room.Database
import androidx.room.RoomDatabase
import just.somebody.templates.appModule.storage.database.entities.ExampleEntity

@Database(
  entities = [ ExampleEntity::class ],
  version  = 1
)
abstract class ExampleDatabase : RoomDatabase()
{
  companion object { val DB_NAME : String = "Example"}
}