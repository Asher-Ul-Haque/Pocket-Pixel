package just.somebody.templates.appModule.storage.dataStore

import androidx.datastore.core.DataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first

/**
 * Operational broker managing interaction bounds targeting the persistent Proto DataStore pipeline.
 *
 * Wraps low-level state mutation behaviors, offering safe transactional updates, value cache clearance,
 * and observational reactive streams to processing layers.
 *
 * @property DATASTORE Direct Jetpack DataStore primitive storage link mapping [AppSettings] components.
 */
class DataStoreManager(private val DATASTORE : DataStore<AppSettings>)
{
  /** Cold stream exposing state notifications whenever updates are safely written onto disk storage. */
  val settingsFlow : Flow<AppSettings> = DATASTORE.data

  /**
   * Commits structural mutations to persistent storage asynchronously in an isolated, atomic transaction.
   *
   * @param NEW_SETTINGS Complete data map replacement payload configuration model.
   */
  suspend fun updateSettings(NEW_SETTINGS : AppSettings)
  {
    DATASTORE.updateData () { NEW_SETTINGS }
  }

  /** Flushes storage contents completely, resetting values safely back to standard default states. */
  suspend fun clearSettings()
  { DATASTORE.updateData { AppSettings() } }

  /**
   * Pulls the current structural snapshot evaluation out from the asynchronous stream pipeline.
   *
   * @return The immediate current instance value configuration of [AppSettings].
   */
  suspend fun getSettings() : AppSettings
  { return settingsFlow.first() }
}