package just.somebody.templates.appModule.storage.dataStore

import androidx.datastore.core.DataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first

class DataStoreManager(private val DATASTORE : DataStore<AppSettings>)
{
  val settingsFlow : Flow<AppSettings> = DATASTORE.data

  suspend fun updateSettings(NEW_SETTINGS : AppSettings)
  {
    DATASTORE.updateData () { NEW_SETTINGS }
  }

  suspend fun clearSettings()
  { DATASTORE.updateData { AppSettings() } }

  suspend fun getSettings() : AppSettings
  { return settingsFlow.first() }
}