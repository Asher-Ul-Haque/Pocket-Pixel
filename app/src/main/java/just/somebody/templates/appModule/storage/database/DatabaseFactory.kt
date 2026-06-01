package just.somebody.templates.appModule.storage.database

import android.content.Context
import androidx.room.Room
import androidx.room.RoomDatabase

/**
 * Factory class responsible for initializing and configuring the application's local Room database builder instance.
 *
 * @property CONTEXT The platform context reference used to resolve asset file path boundaries.
 */
class DatabaseFactory(private val CONTEXT : Context)
{
  /**
   * Configures and prepares a target instance configuration builder for the persistent database stack.
   *
   * Resolves the persistent workspace profile dynamically using explicit absolute path constraints
   * to ensure operational safety across system environments.
   *
   * @return A pre-configured [RoomDatabase.Builder] instance targeting [PixelPocketDB].
   */
  fun create() : RoomDatabase.Builder<PixelPocketDB>
  {
    val appContext = CONTEXT
    val dbFile     = appContext.getDatabasePath(PixelPocketDB.DB_NAME)

    return Room.databaseBuilder(
      context = appContext,
      name    = dbFile.absolutePath)
  }
}