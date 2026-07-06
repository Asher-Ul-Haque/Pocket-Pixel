package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.appModule.storage.LocalAssetManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.data.entities.AchievementEntity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import java.util.Collections

data class GroupedAchievements(
  val gameTitle     : String,
  val achievements  : List<AchievementEntity>)

class AchievementViewModel : ViewModel()
{
  private val dataStore = App.appModule.dataStoreManager
  private val db        = App.appModule.database
    
  val settings: StateFlow<AppSettings> = dataStore.settingsFlow.stateIn(
    scope           = viewModelScope,
    started         = SharingStarted.WhileSubscribed(5000),
    initialValue    = AppSettings())

    val isLoading: StateFlow<Boolean> = App.appModule.isRaSyncing

    private val badgeQueue = Channel<AchievementEntity>(capacity = Channel.UNLIMITED)
    private val queuedIds  = Collections.synchronizedSet(mutableSetOf<Int>())

    init {
        // We removed auto-login on connection recovery to prevent "login already in progress" errors.
        // The user can now manually trigger a login/sync from the Settings screen if needed.
        startBadgeWorker()
    }

    private fun startBadgeWorker() {
        // Allow 4 parallel download workers (non-sequential)
        repeat(4) {
            viewModelScope.launch(Dispatchers.IO) {
                for (achievement in badgeQueue) {
                    try {
                        App.appModule.localAssetManager.downloadToCache(
                            URL = achievement.badgeUrl,
                            FILE_NAME = "badge_${achievement.raId}.png",
                            CATEGORY = LocalAssetManager.CATEGORY_ACHIEVEMENTS
                        )?.let { localUri ->
                            db.achievementDAO().insertAchievement(achievement.copy(badgeUrl = localUri))
                        }
                    } catch (e: Exception) {
                        ForgeLogger.error("Failed to download badge ${achievement.raId}: $e")
                    } finally {
                        queuedIds.remove(achievement.raId)
                    }
                }
            }
        }
    }

    val groupedAchievements: StateFlow<List<GroupedAchievements>> =
      db
        .achievementDAO()
        .getAllAchievements()
        .onEach { list ->
            // Trigger background downloads for missing badges
            list.forEach { achievement ->
                if (!achievement.badgeUrl.startsWith("content://") && !queuedIds.contains(achievement.raId)) {
                    queuedIds.add(achievement.raId)
                    viewModelScope.launch { badgeQueue.send(achievement) }
                }
            }
        }
        .map()
        { list ->
          // - - - Group by game
          list.groupBy { it.raGameId }.map()
          { (raGameId, achievements) ->
            val first = achievements.first()
            GroupedAchievements(first.raGameTitle, achievements)
          }
    }.stateIn(
      scope         = viewModelScope,
      started       = SharingStarted.WhileSubscribed(5000),
      initialValue  = emptyList())

    private val _loginError = MutableStateFlow<String?>(null)
    val loginError: StateFlow<String?> = _loginError

    fun login(USER_NAME: String, password: String)
    {
      viewModelScope.launch()
        {
          _loginError.value = null
          App.appModule.isRaSyncing.value = true
          App.appModule.gameBoy.raLoginWithPassword(USER_NAME, password)
        }
    }

    fun setLoginError(ERROR: String?)
    { 
        _loginError.value = ERROR 
        App.appModule.isRaSyncing.value = false
    }

    fun logout()
    {
      viewModelScope.launch()
      {
        val current = dataStore.getSettings()
        dataStore.updateSettings(current.copy(
            raUsername = "", 
            raToken = "", 
            raAvatarUrl = "",
            raTotalPoints = 0,
            raTotalHardcorePoints = 0,
            assetUrlMapping = emptyMap()
        ))
        App.appModule.gameBoy.raLogout()
        db.achievementDAO().deleteAllAchievements()
        App.appModule.localAssetManager.clearAllCache()
        App.appModule.isRaSyncing.value = false
      }
    }
}
