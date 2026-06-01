package just.somebody.templates.appModule

import android.content.Context
import android.content.res.Configuration
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalConfiguration
import androidx.datastore.core.DataStore
import androidx.datastore.core.DataStoreFactory
import androidx.datastore.dataStoreFile
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.storage.dataStore.AppSettingsSerializer
import just.somebody.templates.appModule.storage.dataStore.DataStoreManager
import just.somebody.templates.appModule.storage.DefaultInternalStorageManager
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
import just.somebody.templates.appModule.storage.DefaultExternalStorageManager
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.appModule.storage.SaveStateManager
import just.somebody.templates.domain.GameBoy
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob

/**
 * Dependency injection contract outlining the global instance topology for the application.
 *
 * Defines access structures for persistent engines, communication managers, presentation routers,
 * and background emulation cores, acting as a single manual injection graph container.
 */
interface AppModuleInterface
{
  /** Client providing connectivity endpoints targeting remote indexing servers. */
  val api                    : Api

  /** Domain access gateway controlling transactions for stored game metadata records. */
  val repo                   : GameRepository

  /** Screen coordinator piping navigation directives across feature boundaries asynchronously. */
  val navigator              : Navigator

  /** Platform checker executing status logic checks for security permissions. */
  val permissionManager      : PermissionManager

  /** Broker processing internal hardware states like battery, thermal levels, or vibrations. */
  val hardwareManager        : HardwareManager

  /** Interface parsing user profile configurations and setting updates asynchronously. */
  val dataStoreManager       : DataStoreManager

  /** Storage interface manipulating private persistent data files or non-persistent cache segments. */
  val internalStorageManager : InternalStorageManager

  /** Storage broker maintaining Storage Access Framework document trees across sessions. */
  val externalStorageManager : ExternalStorageManager

  /** Global application runtime workspace configuration link. */
  val context                : Context

  /** SQLite abstract room database handling tables for games and states. */
  val database               : PixelPocketDB

  /** Ktor transport configuration managing active remote web channels. */
  val networkService         : NetworkService

  /** Location lookup keyword corresponding to the primary external rom directory target tree. */
  val gameRomsKey            : String

  /** Parsing client running queries to cache and present remote cover art images. */
  val boxArtFetcher          : BoxArtFetcher

  /** Isolated core emulation machine processing physical instruction loops. */
  val gameBoy                : GameBoy

  /** Hub mapping physical controller button clicks or thumbstick vectors to the core runtime engine. */
  val gameControllerManager  : GameControllerManager

  /** Workspace supervisor serializing ongoing running emulator states onto local storage files. */
  val saveStateManager       : SaveStateManager

  /** Evaluates if the running device display is in a landscape orientation. */
  @Composable fun isLandscape() : Boolean
}

/**
 * Primary manual dependency injection block allocating lazily evaluated components.
 *
 * @property APP_CONTEXT Platform background context tracking global execution limits.
 */
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
  override val gameControllerManager  : GameControllerManager   by lazy { DefaultGameControllerManager(APP_CONTEXT) }
  override val saveStateManager       : SaveStateManager        by lazy { SaveStateManager(APP_CONTEXT, database.saveStateDAO()) }

  /**
   * Private DataStore instance managing raw persistent mutations of [AppSettings].
   * Configured on an isolated IO thread using a [SupervisorJob] to survive child routine failures.
   */
  private val appSettingsDataStore : DataStore<AppSettings> by lazy ()
  {
    DataStoreFactory.create(
      serializer = AppSettingsSerializer,
      scope      = CoroutineScope(Dispatchers.IO + SupervisorJob())
                           )
    { APP_CONTEXT.dataStoreFile("app-settings.json") }
  }

  @Composable
  public override fun isLandscape() : Boolean
  {
    return LocalConfiguration.current.orientation == Configuration.ORIENTATION_LANDSCAPE
  }
}