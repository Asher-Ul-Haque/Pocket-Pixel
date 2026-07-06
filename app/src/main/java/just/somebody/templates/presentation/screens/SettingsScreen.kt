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
 */
@Composable
fun SettingsScreen(
  VIEW_MODEL : SettingsViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val settings        by VIEW_MODEL.settings.collectAsState()

  SettingsContent(
    CONTROLLER_STATE    = controllerState,
    SETTINGS            = settings,
    ON_REFRESH          = { VIEW_MODEL.updateControllerConnection() },
    ON_DEADZONE_CHANGE  = { VIEW_MODEL.setDeadzone(it) },
    ON_SET_PALETTE      = { VIEW_MODEL.setPalette(it) },
    ON_SET_SHADER       = { VIEW_MODEL.setShader(it) },
    ON_RESCAN           = { VIEW_MODEL.rescan() },
    ON_FACTORY_RESET    = { VIEW_MODEL.factoryReset() },
    ON_MAP_BUTTON       =
      { keyCode, button ->
        VIEW_MODEL.setGamepadButtonMapping(keyCode, button)
      },
    ON_MAP_AXIS         =
      { axis, dir, button ->
        VIEW_MODEL.setGamepadAxisMapping(axis, dir, button)
      },
    ON_TOGGLE_IMMERSIVE   = { VIEW_MODEL.toggleImmersiveMode() },
    ON_TOGGLE_DEFERRED    = { VIEW_MODEL.toggleDeferredSaving() },
    ON_TOGGLE_RA_HARDCORE = { VIEW_MODEL.toggleRaHardcoreMode() },
    ON_RA_LOGIN           = { VIEW_MODEL.raLogin() },
    ON_RA_LOGOUT          = { VIEW_MODEL.raLogout() },
    ON_SET_VOLUME         =
      {
        vol, ch ->
        VIEW_MODEL.setVolume(vol, ch)
      },
    MODIFIER = MODIFIFER)
}

/**
 * Main layout content panel for the application settings screen.
 */
@Composable
fun SettingsContent(
  CONTROLLER_STATE      : GameControllerState,
  SETTINGS              : AppSettings,
  ON_REFRESH            : () -> Unit,
  ON_DEADZONE_CHANGE    : (Float) -> Unit,
  ON_SET_PALETTE        : (Int) -> Unit,
  ON_SET_SHADER         : (Int) -> Unit,
  ON_RESCAN             : () -> Unit,
  ON_FACTORY_RESET      : () -> Unit,
  ON_MAP_BUTTON         : (Int, Buttons?) -> Unit,
  ON_MAP_AXIS           : (Int, Int, Buttons?) -> Unit,
  ON_TOGGLE_IMMERSIVE   : () -> Unit,
  ON_TOGGLE_DEFERRED    : () -> Unit,
  ON_TOGGLE_RA_HARDCORE : () -> Unit,
  ON_RA_LOGIN           : () -> Unit,
  ON_RA_LOGOUT          : () -> Unit,
  ON_SET_VOLUME         : (Float, Int) -> Unit,
  MODIFIER              : Modifier = Modifier)
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
      horizontalAlignment = Alignment.CenterHorizontally,
      contentPadding      = PaddingValues(bottom = 80.dp))
    {
      // - - - Section: Controller
      item()
      {
        SettingsSection(TITLE = stringResource(R.string.controller), ICON = R.drawable.controller)
        {
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            Row(
              modifier              = Modifier.fillMaxWidth(),
              horizontalArrangement = Arrangement.SpaceBetween,
              verticalAlignment     = Alignment.CenterVertically)
            {
              CustomText(stringResource(R.string.device, CONTROLLER_STATE.deviceName), FONT_SIZE = 14)
              IconButton(onClick = ON_REFRESH)
              {
                Icon(
                  painter             = painterResource(R.drawable.redo),
                  contentDescription  = stringResource(R.string.refresh),
                  tint                = GameBoyColors.LightGreen,
                  modifier            = Modifier.size(24.dp))
              }
            }

            CustomText(stringResource(R.string.deadzone, "%.2f".format(SETTINGS.gamepadMapping.deadzone)), FONT_SIZE = 12)
            RetroSlider(
              VALUE           = SETTINGS.gamepadMapping.deadzone,
              ON_VALUE_CHANGE = ON_DEADZONE_CHANGE,
              MODIFIER        = Modifier.fillMaxWidth())

            HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp)

            CustomText(stringResource(R.string.current_bindings), FONT_SIZE = 14)

            // - - - Active Inputs
            val pressedButtons = CONTROLLER_STATE.buttons.filter { it.value }.keys.toList()
            pressedButtons.forEach()
            { keyCode ->
              MappingItem(
                LABEL     = KeyEvent.keyCodeToString(keyCode),
                MAPPED    = SETTINGS.gamepadMapping.buttonToGameBoy[keyCode],
                ON_CLICK  = { showMappingDialog = 0 to keyCode })
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

            CustomText(stringResource(R.string.default_mappings), FONT_SIZE = 14)
            Column(
              modifier = Modifier
                .fillMaxWidth()
                .padding(start = 8.dp))
            {
              CustomText(stringResource(R.string.a_mapping),      FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText(stringResource(R.string.b_mapping),      FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText(stringResource(R.string.start_mapping),  FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText(stringResource(R.string.select_mapping), FONT_SIZE = 12, COLOR = GameBoyColors.Green)
              CustomText(stringResource(R.string.dpad_mapping),   FONT_SIZE = 12, COLOR = GameBoyColors.Green)
            }
          }
        }
      }

      // - - - Section: Visual
      item()
      {
        SettingsSection(TITLE = stringResource(R.string.visual), ICON = R.drawable.paint_blob)
        {
          Column(verticalArrangement = Arrangement.spacedBy(12.dp))
          {
            CustomText(stringResource(R.string.dmg_palettes), FONT_SIZE = 14)
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

            CustomText(stringResource(R.string.shaders), FONT_SIZE = 14)
            val shaderOptions = listOf(
              stringResource(R.string.shader_sharp),
              stringResource(R.string.shader_crt),
              stringResource(R.string.shader_lcd),
              stringResource(R.string.shader_chromatic),
              stringResource(R.string.shader_default))
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
        SettingsSection(TITLE = stringResource(R.string.audio), ICON = R.drawable.speaker)
        {
          val labels = listOf(
            stringResource(R.string.pulse1),
            stringResource(R.string.pulse2),
            stringResource(R.string.wave),
            stringResource(R.string.noise))
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

      // - - - Section: Retro Achievements
      item()
      {
        SettingsSection(TITLE = stringResource(R.string.RA), ICON = R.drawable.trophy)
        {
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            CustomButton(ON_CLICK = ON_TOGGLE_RA_HARDCORE, MODIFIER = Modifier.fillMaxWidth())
            {
              CustomText(
                if (SETTINGS.isRaHardcoreEnabled)  stringResource(R.string.HARDCORE_ON)
                else                               stringResource(R.string.HARDCORE_OFF),
                FONT_SIZE = 14,
                MODIFIER  = Modifier)
            }

            CustomButton(ON_CLICK = ON_RA_LOGIN)
            {
              CustomText(
                stringResource(R.string.RELOGIN),
                FONT_SIZE = 14,
                MODIFIER  = Modifier.fillMaxWidth())
            }

            CustomButton(ON_CLICK = ON_RA_LOGOUT, COLOR = GameBoyColors.Error)
            {
              CustomText(
                stringResource(R.string.LOGOUT),
                FONT_SIZE = 14,
                MODIFIER  = Modifier.fillMaxWidth())
            }
          }
        }
      }

      // -  -- Section: Misc
      item()
      {
        SettingsSection(TITLE = stringResource(R.string.misc), ICON = R.drawable.settings) {
          Column(verticalArrangement = Arrangement.spacedBy(8.dp))
          {
            CustomButton(ON_CLICK = {}, MODIFIER = Modifier.fillMaxWidth())
            { 
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.redo), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(stringResource(R.string.CHANGE), FONT_SIZE = 14, MODIFIER = Modifier) 
              }
            }
            CustomButton(ON_CLICK = ON_TOGGLE_IMMERSIVE, MODIFIER = Modifier.fillMaxWidth())
            {
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.gameboy), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(
                  if (SETTINGS.isImmersiveModeEnabled)  stringResource(R.string.immersive_on)
                  else                                  stringResource(R.string.immersive_off),
                  FONT_SIZE = 14, MODIFIER = Modifier)
              }
            }
            CustomButton(ON_CLICK = ON_TOGGLE_DEFERRED, MODIFIER = Modifier.fillMaxWidth())
            {
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.redo), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(
                  if (SETTINGS.isDeferredSavingEnabled) stringResource(R.string.DEFERRED_SAVING_ON)
                  else                                  stringResource(R.string.DEFERRED_SAVING_OFF),
                  FONT_SIZE = 14, MODIFIER = Modifier)
              }
            }
            CustomButton(ON_RESCAN, MODIFIER = Modifier.fillMaxWidth())
            { 
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.redo), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(stringResource(R.string.RESCAN), FONT_SIZE = 14, MODIFIER = Modifier) 
              }
            }
            CustomButton(
              ON_CLICK = { App.appModule.screenshotManager.openAllScreenshots() },
              MODIFIER = Modifier.fillMaxWidth())
            { 
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.camera), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(stringResource(R.string.VIEW_SCREENSHOTS), FONT_SIZE = 14, MODIFIER = Modifier) 
              }
            }
            CustomButton(ON_CLICK = ON_FACTORY_RESET, MODIFIER = Modifier.fillMaxWidth(), COLOR = GameBoyColors.Error)
            { 
              Row(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Start) 
              {
                Icon(painterResource(R.drawable.trash), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
                Spacer(Modifier.width(12.dp))
                CustomText(stringResource(R.string.FACTORY), FONT_SIZE = 14, MODIFIER = Modifier) 
              }
            }
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
private fun SettingsSection(TITLE: String, ICON: Int, CONTENT: @Composable () -> Unit)
{
  Column(
    modifier = Modifier
      .fillMaxWidth()
      .border(4.dp, GameBoyColors.Green, RectangleShape)
      .padding(16.dp),
    horizontalAlignment = Alignment.Start)
  {
    Row(verticalAlignment = Alignment.CenterVertically) {
      Icon(
        painter = painterResource(ICON),
        contentDescription = null,
        tint = GameBoyColors.LightGreen,
        modifier = Modifier.size(24.dp).padding(end = 8.dp)
      )
      CustomText(
        TITLE,
        FONT_SIZE = 18,
        COLOR     = GameBoyColors.LightGreen,
        MODIFIER  = Modifier.padding(bottom = 4.dp))
    }
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
    title             = { CustomText(stringResource(R.string.map_to_gb)) },
    text              =
      {
        LazyColumn(modifier = Modifier.height(200.dp))
        {
          item()
          {
            TextButton(
              onClick   = { ON_SELECT(null) },
              modifier  = Modifier.fillMaxWidth())
            { CustomText(stringResource(R.string.none)) }
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
      { TextButton(onClick = ON_DISMISS) { CustomText(stringResource(R.string.cancel)) } },
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
    MotionEvent.AXIS_X          -> App.appModule.context.getString(R.string.axis_x)
    MotionEvent.AXIS_Y          -> App.appModule.context.getString(R.string.axis_y)
    MotionEvent.AXIS_Z          -> App.appModule.context.getString(R.string.axis_z)
    MotionEvent.AXIS_RZ         -> App.appModule.context.getString(R.string.axis_rz)
    MotionEvent.AXIS_HAT_X      -> App.appModule.context.getString(R.string.axis_hat_x)
    MotionEvent.AXIS_HAT_Y      -> App.appModule.context.getString(R.string.axis_hat_y)
    MotionEvent.AXIS_LTRIGGER   -> App.appModule.context.getString(R.string.axis_ltrigger)
    MotionEvent.AXIS_RTRIGGER   -> App.appModule.context.getString(R.string.axis_rtrigger)
    else                        -> App.appModule.context.getString(R.string.axis_generic, AXIS)
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
    ON_TOGGLE_DEFERRED  = {},
    ON_TOGGLE_RA_HARDCORE = {},
    ON_RA_LOGIN         = {},
    ON_RA_LOGOUT        = {},
    ON_SET_VOLUME       = { _, _ -> })
}
