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

class AchievementViewModel : ViewModel() {
    private val dataStore = App.appModule.dataStoreManager
    private val db = App.appModule.database
    
    val settings: StateFlow<AppSettings> = dataStore.settingsFlow.stateIn(
        scope = viewModelScope,
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = AppSettings()
    )

    val achievements: StateFlow<List<AchievementEntity>> = db.achievementDAO().getAllAchievements().map { list ->
        list.map { achievement ->
            if (!achievement.badgeUrl.startsWith("content://")) {
                val localUri = App.appModule.localAssetManager.getCachedAsset(
                    url = achievement.badgeUrl,
                    fileName = "badge_${achievement.raId}.png",
                    category = LocalAssetManager.CATEGORY_ACHIEVEMENTS
                )
                if (localUri != achievement.badgeUrl) {
                    viewModelScope.launch {
                        db.achievementDAO().insertAchievement(achievement.copy(badgeUrl = localUri))
                    }
                }
                achievement.copy(badgeUrl = localUri)
            } else {
                achievement
            }
        }
    }.stateIn(
        scope = viewModelScope,
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = emptyList()
    )

    fun login(username: String, password: String) {
        viewModelScope.launch {
            // Note: We don't store the password in DataStore.
            // We initiate login and wait for the token callback.
            App.appModule.gameBoy.raLoginWithPassword(username, password)
        }
    }

    fun logout() {
        viewModelScope.launch {
            val current = dataStore.getSettings()
            dataStore.updateSettings(current.copy(raUsername = "", raToken = "", assetUrlMapping = emptyMap()))
            App.appModule.gameBoy.raLogout()
            db.achievementDAO().deleteAllAchievements()
        }
    }
}
