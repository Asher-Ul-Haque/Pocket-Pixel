package just.somebody.templates.appModule


import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.core.DataStoreFactory
import androidx.datastore.dataStoreFile
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.DefaultExternalStorageManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.AppSettingsSerializer
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.appModule.storage.DefaultInternalStorageManager
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.appModule.storage.InternalStorageManager
import just.somebody.templates.appModule.storage.database.DatabaseFactory
import just.somebody.templates.appModule.storage.database.PixelPocketDB
import just.somebody.templates.data.Api
import just.somebody.templates.data.ApiImpl
import just.somebody.templates.domain.repositories.DefaultGameRepository
import just.somebody.templates.domain.repositories.GameRepository
import just.somebody.templates.appModule.navigation.DefaultNavigator
import just.somebody.templates.presentation.screens.Destination
import just.somebody.templates.appModule.navigation.Navigator
import just.somebody.templates.data.BoxArtFetcher
import just.somebody.templates.data.DefaultBoxArtFetcher
import just.somebody.templates.domain.GameBoy
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob


// - - - add the necessary components from data and domain
interface AppModuleInterface
{
  val api                    : Api
  val repo                   : GameRepository
  val navigator              : Navigator
  val permissionManager      : PermissionManager
  val hardwareManager        : HardwareManager
  val dataStoreManager       : DataStoreManager
  val internalStorageManager : InternalStorageManager
  val externalStorageManager : ExternalStorageManager
  val context                : Context
  val database               : PixelPocketDB
  val networkService         : NetworkService
  val gameRomsKey            : String
  val boxArtFetcher          : BoxArtFetcher
  val gameBoy                : GameBoy
}

class AppModule(private val APP_CONTEXT : Context) : AppModuleInterface
{
  override val context                : Context                 by lazy { APP_CONTEXT }
  override val api                    : Api                     by lazy { ApiImpl(); }
  override val repo                   : GameRepository          by lazy { DefaultGameRepository(database.gameDAO());}
  override val navigator              : Navigator               by lazy { DefaultNavigator(startDestination = Destination.Home) }
  override val hardwareManager        : HardwareManager         by lazy { DefaultHardwareManager(APP_CONTEXT) }
  override val permissionManager      : PermissionManager       by lazy { DefaultPermissionManager() }
  override val dataStoreManager       : DataStoreManager        by lazy { DataStoreManager(appSettingsDataStore) }
  override val internalStorageManager : InternalStorageManager  by lazy { DefaultInternalStorageManager(APP_CONTEXT) }
  override val externalStorageManager : ExternalStorageManager  by lazy { DefaultExternalStorageManager(APP_CONTEXT, dataStoreManager) }
  override val networkService         : NetworkService          by lazy { NetworkService() }
  override val database               : PixelPocketDB           by lazy { DatabaseFactory(APP_CONTEXT).create()
    .fallbackToDestructiveMigration(true)
    .build() }
  override val gameRomsKey            : String                  by lazy { "GAME_BOY_ROMS" }
  override val boxArtFetcher          : BoxArtFetcher           by lazy { DefaultBoxArtFetcher(internalStorageManager, networkService) }
  override val gameBoy                : GameBoy                 by lazy { GameBoy() }


  private val appSettingsDataStore : DataStore<AppSettings> by lazy ()
  {
    DataStoreFactory.create(
      serializer = AppSettingsSerializer,
      scope      = CoroutineScope(Dispatchers.IO + SupervisorJob())
    )
    { APP_CONTEXT.dataStoreFile("app-settings.json") }
  }
}