package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.storage.LocalAssetManager
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.data.entities.AchievementEntity
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

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

    val groupedAchievements: StateFlow<List<GroupedAchievements>> =
      db
        .achievementDAO()
        .getAllAchievements()
        .map()
        { list ->
          // - - - Group by game
          list.groupBy { it.raGameId }.map()
          { (raGameId, achievements) ->
            val first = achievements.first()
            
            // Trigger background downloads for missing badges
            achievements.forEach { achievement ->
                if (!achievement.badgeUrl.startsWith("content://")) {
                    viewModelScope.launch {
                        App.appModule.localAssetManager.downloadToCache(
                            URL = achievement.badgeUrl,
                            FILE_NAME = "badge_${achievement.raId}.png",
                            CATEGORY = LocalAssetManager.CATEGORY_ACHIEVEMENTS
                        )?.let { localUri ->
                            db.achievementDAO().insertAchievement(achievement.copy(badgeUrl = localUri))
                        }
                    }
                }
            }
            
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
          App.appModule.gameBoy.raLoginWithPassword(USER_NAME, password)
        }
    }

    fun setLoginError(ERROR: String?)
    { _loginError.value = ERROR }

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
      }
    }

    fun resync() {
        viewModelScope.launch {
            db.achievementDAO().deleteAllAchievements()
            App.appModule.localAssetManager.clearAllCache()
            App.appModule.gameBoy.raSyncProfile()
        }
    }
}