package just.somebody.templates

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.size
import androidx.compose.ui.Modifier
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.unit.dp
import just.somebody.templates.appModule.GameControllerState
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.presentation.screens.SettingsContent
import org.junit.Rule
import org.junit.Test

class SettingsLayoutTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun settingsScreen_displaysAllSections() {
        composeTestRule.setContent {
            Box(modifier = Modifier.size(width = 411.dp, height = 891.dp)) {
                SettingsContent(
									CONTROLLER_STATE = GameControllerState(),
									SETTINGS = AppSettings(),
									ON_REFRESH = {},
									ON_DEADZONE_CHANGE = {},
									ON_SET_PALETTE = {},
									ON_SET_SHADER = {},
									ON_RESCAN = {},
									ON_FACTORY_RESET = {},
									ON_MAP_BUTTON = { _, _ -> },
									ON_MAP_AXIS = { _, _, _ -> },
									ON_TOGGLE_IMMERSIVE = {},
									ON_SET_VOLUME = { _, _ -> }
                )
            }
        }

        // Verify sections by their titles
        composeTestRule.onNodeWithText("Controller").assertIsDisplayed()
        composeTestRule.onNodeWithText("Visual").assertIsDisplayed()
        composeTestRule.onNodeWithText("Audio").assertIsDisplayed()
        composeTestRule.onNodeWithText("Misc").assertIsDisplayed()
    }
}
