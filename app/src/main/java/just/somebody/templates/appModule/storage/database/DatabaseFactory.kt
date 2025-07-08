package just.somebody.templates.appModule.storage.database

import android.content.Context
import androidx.room.Room
import androidx.room.RoomDatabase

class DatabaseFactory(private val CONTEXT : Context)
{
  fun create() : RoomDatabase.Builder<ExampleDatabase>
  {
    val appContext = CONTEXT
    val dbFile     = appContext.getDatabasePath(ExampleDatabase.DB_NAME)

    return Room.databaseBuilder(
      context = appContext,
      name    = dbFile.absolutePath
    )
  }
}