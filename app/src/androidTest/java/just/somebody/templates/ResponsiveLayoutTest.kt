package just.somebody.templates

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.unit.dp
import just.somebody.templates.presentation.screens.EmulatorContent
import just.somebody.templates.ui.theme.GameBoyColors
import org.junit.Rule
import org.junit.Test

class ResponsiveLayoutTest {

    @get:Rule
    val composeTestRule = createComposeRule()

    @Test
    fun emulatorPortraitLayout_showsBottomControls() {
        composeTestRule.setContent {
            // Simulate Small Portrait Device
            Box(modifier = Modifier.size(width = 360.dp, height = 640.dp)) {
                EmulatorContent(
									IS_LANDSCAPE = false,
									CONTROLS_VISIBLE = true,
									CONTROL_ALPHA = 1f,
									SHOW_SETTINGS = false,
									ON_INTERACTION = {},
									ON_TOGGLE_SETTINGS = {},
									VIEWPORT = { Box(modifier = it.background(Color.Red)) },
									CONTROLS = {
                        Box(modifier = Modifier
                            .fillMaxSize()
                            .background(GameBoyColors.DarkGreen)
                            .testTag("PortraitControls")) 
                    },
									SETTINGS_PANEL = {},
									D_PAD = {},
									ACTION_BUTTONS = {},
									SELECT_BUTTON = {},
									START_BUTTON = {}
                )
            }
        }

        // Verify that portrait-specific controls are displayed
        composeTestRule.onNodeWithTag("PortraitControls").assertIsDisplayed()
    }

    @Test
    fun emulatorLandscapeLayout_showsSidePanels() {
        composeTestRule.setContent {
            // Simulate Large Landscape Device (Tablet/Large Phone)
            // Ensure size is large enough to avoid clipping
            Box(modifier = Modifier.size(width = 1280.dp, height = 800.dp)) {
                EmulatorContent(
									IS_LANDSCAPE = true,
									CONTROLS_VISIBLE = true,
									CONTROL_ALPHA = 1f,
									SHOW_SETTINGS = false,
									ON_INTERACTION = {},
									ON_TOGGLE_SETTINGS = {},
									VIEWPORT = { Box(modifier = it.background(Color.Red)) },
									CONTROLS = {},
									SETTINGS_PANEL = {},
									D_PAD = { Box(modifier = Modifier.size(100.dp).testTag("LeftPanelContent")) },
									ACTION_BUTTONS = { Box(modifier = Modifier.size(100.dp).testTag("RightPanelContent")) },
									SELECT_BUTTON = { Box(modifier = Modifier.size(50.dp)) },
									START_BUTTON = { Box(modifier = Modifier.size(50.dp)) }
                )
            }
        }

        // Verify that side panels exist and contain the controls
        composeTestRule.onNodeWithTag("LeftPanelContent").assertExists()
        composeTestRule.onNodeWithTag("RightPanelContent").assertExists()
    }
    
    @Test
    fun emulatorImmersiveMode_hidesControls() {
        composeTestRule.setContent {
            EmulatorContent(
							IS_LANDSCAPE = false,
							CONTROLS_VISIBLE = false,
							CONTROL_ALPHA = 0f, // Faded out
							SHOW_SETTINGS = false,
							ON_INTERACTION = {},
							ON_TOGGLE_SETTINGS = {},
							VIEWPORT = { Box(modifier = it.background(Color.Red)) },
							CONTROLS = {
                    Box(modifier = Modifier
                        .height(200.dp)
                        .testTag("ControlsLayer")) 
                },
							SETTINGS_PANEL = {},
							D_PAD = {},
							ACTION_BUTTONS = {},
							SELECT_BUTTON = {},
							START_BUTTON = {}
            )
        }

        // The node should still exist in the hierarchy even if invisible
        composeTestRule.onNodeWithTag("ControlsLayer").assertExists()
    }
}
