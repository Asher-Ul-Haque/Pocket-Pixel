package just.somebody.templates.presentation.screens

import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.core.graphics.toColorInt
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.models.PRESET_PALETTES
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.RetroSlider
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.DeviceSizePreviews
import just.somebody.templates.appModule.storage.dataStore.AppSettings
import just.somebody.templates.appModule.GameControllerState

/**
 * Root state wrapper composable for the application configuration panel.
 *
 * It collects reactive event feeds from both the active external gamepad hardware manager and persistent local data
 * files, piping the decoupled state directly into an un-hoisted content rendering terminal.
 *
 * @param VIEW_MODEL State supervisor tracking global preference parameters and controller assignment mutations.
 * @param MODIFIFER [Modifier] used to establish positional layout bounds or boundary dimensions.
 */
@Composable
fun SettingsScreen(
  VIEW_MODEL : SettingsViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val settings        by VIEW_MODEL.settings.collectAsState()

  SettingsContent(
    CONTROLLER_STATE   = controllerState,
    SETTINGS          = settings,
    ON_REFRESH         = { VIEW_MODEL.updateControllerConnection() },
    ON_DEADZONE_CHANGE  = { VIEW_MODEL.setDeadzone(it) },
    ON_SET_PALETTE      = { VIEW_MODEL.setPalette(it) },
    ON_SET_SHADER       = { VIEW_MODEL.setShader(it) },
    ON_RESCAN          = { VIEW_MODEL.rescan() },
    ON_FACTORY_RESET    = { VIEW_MODEL.factoryReset() },
    ON_MAP_BUTTON       =
      { keyCode, button ->
        VIEW_MODEL.setGamepadButtonMapping(keyCode, button)
      },
    ON_MAP_AXIS         =
      { axis, dir, button ->
        VIEW_MODEL.setGamepadAxisMapping(axis, dir, button)
      },
    ON_TOGGLE_IMMERSIVE = { VIEW_MODEL.toggleImmersiveMode() },
    ON_SET_VOLUME       =
      {
        vol, ch ->
        VIEW_MODEL.setVolume(vol, ch)
      },
    MODIFIER = MODIFIFER)
}

/**
 * Main layout content panel for the application settings screen.
 *
 * Assembles standalone, pixel-styled layout modules divided explicitly into gamepad input assignments,
 * retro visual color spaces, individual sound channel tracking systems, and destructive factory initialization helpers.
 *
 * @param CONTROLLER_STATE Live telemetry structure tracking active controller identifiers and hardware clicks.
 * @param SETTINGS Persistent global profile configuration preferences.
 * @param ON_REFRESH Dispatch lambda to trigger a manual connection check for external gamepad inputs.
 * @param ON_DEADZONE_CHANGE State mutator processing updated thumbstick coordinate filter limits.
 * @param ON_SET_PALETTE Selector callback updating the console's default color lookup index map.
 * @param ON_SET_SHADER Selector callback changing the active post-processing visualization layer.
 * @param ON_RESCAN Utility intent dispatch executing a storage directory scan for new ROM structures.
 * @param ON_FACTORY_RESET Safety prompt callback designed to wipe clean all local configurations and storage paths.
 * @param ON_MAP_BUTTON Input pipeline matching a hardware key code directly onto an emulation key entry.
 * @param ON_MAP_AXIS Input pipeline matching specific analog directional sweeps onto an emulation key entry.
 * @param ON_TOGGLE_IMMERSIVE Display mode override function that handles window status bar flags.
 * @param ON_SET_VOLUME Mixer function assigning amplification scales directly across independent audio channels.
 * @param MODIFIER [Modifier] used to arrange outer container boundaries.
 */
@Composable
fun SettingsContent(
  CONTROLLER_STATE    : GameControllerState,
  SETTINGS            : AppSettings,
  ON_REFRESH          : () -> Unit,
  ON_DEADZONE_CHANGE  : (Float) -> Unit,
  ON_SET_PALETTE      : (Int) -> Unit,
  ON_SET_SHADER       : (Int) -> Unit,
  ON_RESCAN           : () -> Unit,
  ON_FACTORY_RESET    : () -> Unit,
  ON_MAP_BUTTON       : (Int, Buttons?) -> Unit,
  ON_MAP_AXIS         : (Int, Int, Buttons?) -> Unit,
  ON_TOGGLE_IMMERSIVE : () -> Unit,
  ON_SET_VOLUME       : (Float, Int) -> Unit,
  MODIFIER            : Modifier = Modifier)
{
  var showMappingDialog by remember { mutableStateOf<Pair<Int, Int?>?>(null) }

  Box(modifier = MODIFIER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen)
    .padding(16.dp))
  {
    LazyColumn(
      modifier            = Modifier.fillMaxSize(),
      verticalArrangement = Arrangement.spacedBy(16.dp),
      horizontalAlignment = Alignment.CenterHorizontally)
    {
      // - - - Section: Controller
      item()
      {
        SettingsSection(TITLE = "Controller")
        {
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            Row(
              modifier              = Modifier.fillMaxWidth(),
              horizontalArrangement = Arrangement.SpaceBetween,
              verticalAlignment     = Alignment.CenterVertically)
            {
              CustomText("Device: ${CONTROLLER_STATE.deviceName}", FONT_SIZE = 14)
              IconButton(onClick = ON_REFRESH)
              {
                Icon(
                  painter             = painterResource(R.drawable.redo),
                  contentDescription  = "Refresh",
                  tint                = GameBoyColors.LightGreen,
                  modifier            = Modifier.size(24.dp))
              }
            }

            CustomText("Deadzone: ${"%.2f".format(SETTINGS.gamepadMapping.deadzone)}", FONT_SIZE = 12)
            RetroSlider(
              VALUE           = SETTINGS.gamepadMapping.deadzone,
              ON_VALUE_CHANGE = ON_DEADZONE_CHANGE,
              MODIFIER        = Modifier.fillMaxWidth())

            HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp)

            CustomText("Current Bindings (Click to map):", FONT_SIZE = 14)

            // - - - Active Inputs
            val pressedButtons = CONTROLLER_STATE.buttons.filter { it.value }.keys.toList()
            pressedButtons.forEach()
            { keyCode ->
              MappingItem(
                LABEL   = KeyEvent.keyCodeToString(keyCode),
                MAPPED  = SETTINGS.gamepadMapping.buttonToGameBoy[keyCode],
                ON_CLICK = { showMappingDialog = 0 to keyCode })
            }

            val activeAxes = CONTROLLER_STATE.axes.filter()
            { Math.abs(it.value) > SETTINGS.gamepadMapping.deadzone }
            activeAxes.forEach()
            { (axis, value) ->
              val direction =
                if (value > 0)   1
                else            -1
              MappingItem(
                LABEL   = "${getAxisName(axis)} ${
                  if (direction > 0) "+" 
                  else               "-"}",
                MAPPED  = SETTINGS.gamepadMapping.axisToGameBoy[axis]?.get(direction),
                ON_CLICK = { showMappingDialog = (
                  if (direction > 0) 1
                  else            2) to axis })
            }

            HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp)

            CustomText("Default Mappings (Reference):", FONT_SIZE = 14)
            Column(
              modifier = Modifier
                .fillMaxWidth()
                .padding(start = 8.dp))
            {
              CustomText("A Button -> GB A",                    FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText("B/X Button -> GB B",                  FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText("Start Button -> GB Start",            FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText("Select Button -> GB Select",          FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText("D-Pad / Left Stick -> GB Directions", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
            }
          }
        }
      }

      // - - - Section: Visual
      item()
      {
        SettingsSection(TITLE = "Visual")
        {
          Column(verticalArrangement = Arrangement.spacedBy(12.dp))
          {
            CustomText("DMG Palettes", FONT_SIZE = 14)
            PRESET_PALETTES.forEachIndexed()
            { index, palette ->
              Row(
                modifier = Modifier
                  .fillMaxWidth()
                  .clickable { ON_SET_PALETTE(index) }
                  .background(
                    if (SETTINGS.paletteIndex == index) GameBoyColors.MediumGreen
                    else                                Color.Transparent)
                  .padding(4.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment     = Alignment.CenterVertically)
              {
                CustomText(palette.name, FONT_SIZE = 12, MODIFIER = Modifier.weight(1f))
                PalettePreview(palette.colors)
              }
            }

            HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 2.dp)

            CustomText("Shaders", FONT_SIZE = 14)
            val shaderOptions = listOf("Sharp Retro", "CRT", "LCD", "Chromatic Aberration", "Default")
            shaderOptions.forEachIndexed()
            { index, shader ->
              CustomText(
                shader,
                FONT_SIZE = 12,
                MODIFIER = Modifier
                  .fillMaxWidth()
                  .clickable { ON_SET_SHADER(index) }
                  .background(
                    if (SETTINGS.shaderIndex == index)  GameBoyColors.MediumGreen
                    else                                Color.Transparent)
                  .padding(8.dp))
            }
          }
        }
      }

      // - - - Section: Audio
      item()
      {
        SettingsSection(TITLE = "Audio")
        {
          val labels = listOf("Pulse 1", "Pulse 2", "Wave  ", "Noise  ")
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            labels.forEachIndexed()
            { index, label ->
              Column()
              {
                CustomText(label, FONT_SIZE = 12)
                RetroSlider(
                  VALUE           = SETTINGS.channelVolume[index],
                  ON_VALUE_CHANGE = { ON_SET_VOLUME(it, index) },
                  MODIFIER        = Modifier.fillMaxWidth())
              }
            }
          }
        }
      }

      // -  -- Section: Misc
      item()
      {
        SettingsSection(TITLE = "Misc") {
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            CustomButton(ON_CLICK = {}, MODIFIER = Modifier.fillMaxWidth())
            { CustomText(stringResource(R.string.CHANGE)) }
            CustomButton(ON_CLICK = ON_TOGGLE_IMMERSIVE, MODIFIER = Modifier.fillMaxWidth())
            {
              CustomText(
                if (SETTINGS.isImmersiveModeEnabled)  "Immersive Mode: ON"
                else                                  "Immersive Mode: OFF")
            }
            CustomButton(ON_CLICK = ON_RESCAN, MODIFIER = Modifier.fillMaxWidth())
            { CustomText(stringResource(R.string.RESCAN)) }
            CustomButton(ON_CLICK = ON_FACTORY_RESET, MODIFIER = Modifier.fillMaxWidth(), COLOR = GameBoyColors.Error)
            { CustomText(stringResource(R.string.FACTORY)) }
          }
        }
      }
    }
  }

  showMappingDialog?.let()
  { (type, id) ->
    MappingDialog(
      ON_DISMISS = { showMappingDialog = null },
      ON_SELECT  = { gbButton ->
        when (type)
        {
          0 -> ON_MAP_BUTTON(id!!, gbButton)
          1 -> ON_MAP_AXIS(id!!, 1, gbButton)
          2 -> ON_MAP_AXIS(id!!, -1, gbButton)
        }
        showMappingDialog = null
      })
  }
}

/**
 * Standard utility card panel used to group preference blocks with uniform borders.
 */
@Composable
private fun SettingsSection(TITLE: String, CONTENT: @Composable () -> Unit)
{
  Column(
    modifier = Modifier
      .fillMaxWidth()
      .border(4.dp, GameBoyColors.Green, RectangleShape)
      .padding(16.dp),
    horizontalAlignment = Alignment.Start)
  {
    CustomText(
      TITLE,
      FONT_SIZE = 18,
      COLOR     = GameBoyColors.LightGreen,
      MODIFIER  = Modifier.padding(bottom = 4.dp))
    HorizontalDivider(
      thickness = 1.dp,
      color     = GameBoyColors.MediumGreen,
      modifier  = Modifier.fillMaxWidth(0.3f))
    Spacer(modifier = Modifier.height(12.dp))
    CONTENT()
  }
}

/**
 * Specialized list row displaying an active input binding.
 */
@Composable
private fun MappingItem(LABEL: String, MAPPED: Buttons?, ON_CLICK: () -> Unit)
{
  Row(
    modifier = Modifier
      .fillMaxWidth()
      .clickable { ON_CLICK() }
      .background(GameBoyColors.MediumGreen)
      .padding(8.dp),
    horizontalArrangement = Arrangement.SpaceBetween,
    verticalAlignment     = Alignment.CenterVertically)
  {
    CustomText(LABEL, FONT_SIZE = 12)
    CustomText(MAPPED?.name ?: "None", COLOR = GameBoyColors.Green, FONT_SIZE = 12)
  }
}

/**
 * Overlay selection dialog listing all target console buttons available for mapping.
 */
@Composable
private fun MappingDialog(ON_DISMISS: () -> Unit, ON_SELECT: (Buttons?) -> Unit)
{
  AlertDialog(
    onDismissRequest  = ON_DISMISS,
    title             = { CustomText("Map to Game Boy") },
    text              =
      {
        LazyColumn(modifier = Modifier.height(200.dp))
        {
          item()
          {
            TextButton(
              onClick   = { ON_SELECT(null) },
              modifier  = Modifier.fillMaxWidth())
            { CustomText("None") }
        }
        items(Buttons.entries)
        { button ->
          TextButton(
            onClick   = { ON_SELECT(button) },
            modifier  = Modifier.fillMaxWidth())
          { CustomText(button.name) }
        }
      }
    },
    confirmButton = {},
    dismissButton =
      { TextButton(onClick = ON_DISMISS) { CustomText("Cancel") } },
    containerColor  = GameBoyColors.DarkGreen,
    shape           = RectangleShape)
}

/**
 * Horizontal block rendering individual colors for a palette option preview row.
 */
@Composable
private fun PalettePreview(COLORS: List<String>)
{
  Row(horizontalArrangement = Arrangement.spacedBy(4.dp))
  {
    COLORS.forEach()
    { colorHex ->
      Box(
        modifier = Modifier
          .size(16.dp)
          .background(Color(colorHex.toColorInt())))
    }
  }
}

/**
 * Maps hardware motion axis IDs to clear engineering string names for labels.
 */
private fun getAxisName(AXIS: Int): String
{
  return when (AXIS)
  {
    MotionEvent.AXIS_X          -> "X (Left Stick H)"
    MotionEvent.AXIS_Y          -> "Y (Left Stick V)"
    MotionEvent.AXIS_Z          -> "Z (Right Stick H)"
    MotionEvent.AXIS_RZ         -> "RZ (Right Stick V)"
    MotionEvent.AXIS_HAT_X      -> "HAT_X (D-Pad H)"
    MotionEvent.AXIS_HAT_Y      -> "HAT_Y (D-Pad V)"
    MotionEvent.AXIS_LTRIGGER   -> "L-Trigger"
    MotionEvent.AXIS_RTRIGGER   -> "R-Trigger"
    else                        -> "Axis $AXIS"
  }
}

/**
 * Fixed design-time preview function mock-mapping the configuration structure.
 */
@DeviceSizePreviews
@Composable
private fun SettingsPreview()
{
  SettingsContent(
    CONTROLLER_STATE    = GameControllerState(),
    SETTINGS            = AppSettings(),
    ON_REFRESH          = {},
    ON_DEADZONE_CHANGE  = {},
    ON_SET_PALETTE      = {},
    ON_SET_SHADER       = {},
    ON_RESCAN           = {},
    ON_FACTORY_RESET    = {},
    ON_MAP_BUTTON       = { _, _ -> },
    ON_MAP_AXIS         = { _, _, _ -> },
    ON_TOGGLE_IMMERSIVE = {},
    ON_SET_VOLUME       = { _, _ -> })
}