package just.somebody.templates.appModule


import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.core.DataStoreFactory
import androidx.datastore.dataStoreFile
import just.somebody.templates.appModule.nativeBridge.NativeBridge
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.DefaultExternalStorageManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.AppSettingsSerializer
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.appModule.storage.DefaultInternalStorageManager
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.appModule.storage.InternalStorageManager
import just.somebody.templates.appModule.storage.database.DatabaseFactory
import just.somebody.templates.appModule.storage.database.ExampleDatabase
import just.somebody.templates.data.Api
import just.somebody.templates.data.ApiImpl
import just.somebody.templates.data.RepositoryImpl
import just.somebody.templates.domain.Repository
import just.somebody.templates.presentation.screens.DefaultNavigator
import just.somebody.templates.presentation.screens.Destination
import just.somebody.templates.presentation.screens.Navigator
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob


// - - - add the necessary components from data and domain
interface AppModuleInterface
{
  val api                    : Api
  val repo                   : Repository
  val navigator              : Navigator
  val permissionManager      : PermissionManager
  val hardwareManager        : HardwareManager
  val dataStoreManager       : DataStoreManager
  val internalStorageManager : InternalStorageManager
  val externalStorageManager : ExternalStorageManager
  val context                : Context
  val database               : ExampleDatabase
  val networkService         : NetworkService
  val nativeBridge           : NativeBridge
}

class AppModule(private val APP_CONTEXT : Context) : AppModuleInterface
{
  override val context                : Context                 by lazy { APP_CONTEXT }
  override val api                    : Api                     by lazy { ApiImpl(); }
  override val repo                   : Repository              by lazy { RepositoryImpl(this.api);}
  override val navigator              : Navigator               by lazy { DefaultNavigator(startDestination = Destination.ScreenA) }
  override val hardwareManager        : HardwareManager         by lazy { DefaultHardwareManager(APP_CONTEXT) }
  override val permissionManager      : PermissionManager       by lazy { DefaultPermissionManager() }
  override val dataStoreManager       : DataStoreManager        by lazy { DataStoreManager(appSettingsDataStore) }
  override val internalStorageManager : InternalStorageManager  by lazy { DefaultInternalStorageManager(APP_CONTEXT) }
  override val externalStorageManager : ExternalStorageManager  by lazy { DefaultExternalStorageManager(APP_CONTEXT, dataStoreManager) }
  override val networkService         : NetworkService          by lazy { NetworkService() }
  override val database               : ExampleDatabase         by lazy { DatabaseFactory(APP_CONTEXT).create().build() }
  override val nativeBridge           : NativeBridge            by lazy { NativeBridge() }

  private val appSettingsDataStore : DataStore<AppSettings> by lazy ()
  {
    DataStoreFactory.create(
      serializer = AppSettingsSerializer,
      scope      = CoroutineScope(Dispatchers.IO + SupervisorJob())
    ) { APP_CONTEXT.dataStoreFile("app-settings.json") }
  }
}