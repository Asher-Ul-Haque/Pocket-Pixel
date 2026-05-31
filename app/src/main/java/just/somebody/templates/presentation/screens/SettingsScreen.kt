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

@Composable
fun SettingsScreen(
  VIEW_MODEL : SettingsViewModel,
  MODIFIFER  : Modifier = Modifier
)
{
  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val settings by VIEW_MODEL.settings.collectAsState()
  
  var showMappingDialog by remember { mutableStateOf<Pair<Int, Int?>?>(null) } 

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen)
    .padding(16.dp))
  {
    LazyColumn(
      modifier = Modifier.fillMaxSize(),
      verticalArrangement = Arrangement.spacedBy(16.dp),
      horizontalAlignment = Alignment.CenterHorizontally
    ) {
      // --- Section: Controller ---
      item {
          SettingsSection(title = "Controller") {
              Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                  Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically) {
                      CustomText("Device: ${controllerState.deviceName}", FONT_SIZE = 14)
                      IconButton(onClick = { VIEW_MODEL.updateControllerConnection() }) {
                          Icon(
                              painter = painterResource(R.drawable.redo),
                              contentDescription = "Refresh",
                              tint = GameBoyColors.LightGreen,
                              modifier = Modifier.size(24.dp)
                          )
                      }
                  }
                  
                  CustomText("Deadzone: ${"%.2f".format(settings.gamepadMapping.deadzone)}", FONT_SIZE = 12)
                  RetroSlider(
                      VALUE = settings.gamepadMapping.deadzone,
                      ON_VALUE_CHANGE = { VIEW_MODEL.setDeadzone(it) },
                      MODIFIER = Modifier.fillMaxWidth()
                  )

                  HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp)
                  
                  CustomText("Current Bindings (Click to map):", FONT_SIZE = 14)

                  // Active Inputs
                  val pressedButtons = controllerState.buttons.filter { it.value }.keys.toList()
                  pressedButtons.forEach { keyCode ->
                      MappingItem(
                          label = KeyEvent.keyCodeToString(keyCode),
                          mapped = settings.gamepadMapping.buttonToGameBoy[keyCode],
                          onClick = { showMappingDialog = 0 to keyCode }
                      )
                  }

                  val activeAxes = controllerState.axes.filter { Math.abs(it.value) > settings.gamepadMapping.deadzone }
                  activeAxes.forEach { (axis, value) ->
                      val direction = if (value > 0) 1 else -1
                      MappingItem(
                          label = "${getAxisName(axis)} ${if (direction > 0) "+" else "-"}",
                          mapped = settings.gamepadMapping.axisToGameBoy[axis]?.get(direction),
                          onClick = { showMappingDialog = (if (direction > 0) 1 else 2) to axis }
                      )
                  }

                  HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp)

                  CustomText("Default Mappings (Reference):", FONT_SIZE = 14)
                  Column(modifier = Modifier.fillMaxWidth().padding(start = 8.dp)) {
                      CustomText("A Button -> GB A", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
                      CustomText("B/X Button -> GB B", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
                      CustomText("Start Button -> GB Start", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
                      CustomText("Select Button -> GB Select", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
                      CustomText("D-Pad / Left Stick -> GB Directions", FONT_SIZE = 12, COLOR = GameBoyColors.Green)
                  }
              }
          }
      }

      // --- Section: Visual ---
      item {
          SettingsSection(title = "Visual") {
              Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
                  CustomText("DMG Palettes", FONT_SIZE = 14)
                  PRESET_PALETTES.forEachIndexed { index, palette ->
                      Row(
                          modifier = Modifier
                              .fillMaxWidth()
                              .clickable { VIEW_MODEL.setPalette(index) }
                              .background(if (settings.paletteIndex == index) GameBoyColors.MediumGreen else Color.Transparent)
                              .padding(4.dp),
                          horizontalArrangement = Arrangement.SpaceBetween,
                          verticalAlignment = Alignment.CenterVertically
                      ) {
                          CustomText(palette.name, FONT_SIZE = 12, MODIFIER = Modifier.weight(1f))
                          PalettePreview(palette.colors)
                      }
                  }

                  HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 2.dp)
                  
                  CustomText("Shaders", FONT_SIZE = 14)
                  val shaderOptions = listOf("Sharp Retro", "CRT", "LCD", "Chromatic Aberration", "Default")
                  shaderOptions.forEachIndexed { index, shader ->
                      CustomText(
                          shader,
                          FONT_SIZE = 12,
                          MODIFIER = Modifier
                              .fillMaxWidth()
                              .clickable { VIEW_MODEL.setShader(index) }
                              .background(if (settings.shaderIndex == index) GameBoyColors.MediumGreen else Color.Transparent)
                              .padding(8.dp)
                      )
                  }
              }
          }
      }

      // --- Section: Audio ---
      item {
          SettingsSection(title = "Audio") {
              val labels = listOf("CH1", "CH2", "Wave", "Noise")
              Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                  labels.forEachIndexed { index, label ->
                      Column {
                          CustomText(label, FONT_SIZE = 12)
                          RetroSlider(
                              VALUE = settings.channelVolume[index + 1],
                              ON_VALUE_CHANGE = { VIEW_MODEL.setVolume(it, index + 1) },
                              MODIFIER = Modifier.fillMaxWidth()
                          )
                      }
                  }
              }
          }
      }

      // --- Section: Misc ---
      item {
          SettingsSection(title = "Misc") {
              Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                  val pickDirectory = App.appModule.externalStorageManager.DirectoryPickerLauncher("GAME_BOY_ROMS") { uri ->
                      if (uri != null) VIEW_MODEL.rescan()
                  }
                  
                  CustomButton(ON_CLICK = pickDirectory, MODIFIER = Modifier.fillMaxWidth()) {
                      CustomText(stringResource(R.string.CHANGE))
                  }
                  CustomButton(ON_CLICK = { VIEW_MODEL.toggleImmersiveMode() }, MODIFIER = Modifier.fillMaxWidth()) {
                      CustomText(if (settings.isImmersiveModeEnabled) "Immersive Mode: ON" else "Immersive Mode: OFF")
                  }
                  CustomButton(ON_CLICK = { VIEW_MODEL.rescan() }, MODIFIER = Modifier.fillMaxWidth()) {
                      CustomText(stringResource(R.string.RESCAN))
                  }
                  CustomButton(ON_CLICK = { VIEW_MODEL.factoryReset() }, MODIFIER = Modifier.fillMaxWidth(), COLOR = GameBoyColors.Error) {
                      CustomText(stringResource(R.string.FACTORY))
                  }
              }
          }
      }
    }
  }

  showMappingDialog?.let { (type, id) ->
      MappingDialog(
          onDismiss = { showMappingDialog = null },
          onSelect = { gbButton ->
              when (type) {
                  0 -> VIEW_MODEL.setGamepadButtonMapping(id!!, gbButton)
                  1 -> VIEW_MODEL.setGamepadAxisMapping(id!!, 1, gbButton)
                  2 -> VIEW_MODEL.setGamepadAxisMapping(id!!, -1, gbButton)
              }
              showMappingDialog = null
          }
      )
  }
}

@Composable
private fun SettingsSection(title: String, content: @Composable () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .border(4.dp, GameBoyColors.Green, RectangleShape)
            .padding(16.dp),
        horizontalAlignment = Alignment.Start
    ) {
        CustomText(title, FONT_SIZE = 18, COLOR = GameBoyColors.LightGreen, MODIFIER = Modifier.padding(bottom = 4.dp))
        HorizontalDivider(thickness = 1.dp, color = GameBoyColors.MediumGreen, modifier = Modifier.fillMaxWidth(0.3f))
        Spacer(modifier = Modifier.height(12.dp))
        content()
    }
}

@Composable
private fun MappingItem(label: String, mapped: Buttons?, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onClick() }
            .background(GameBoyColors.MediumGreen)
            .padding(8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        CustomText(label, FONT_SIZE = 12)
        CustomText(mapped?.name ?: "None", COLOR = GameBoyColors.Green, FONT_SIZE = 12)
    }
}

@Composable
private fun MappingDialog(onDismiss: () -> Unit, onSelect: (Buttons?) -> Unit) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { CustomText("Map to Game Boy") },
        text = {
            LazyColumn(modifier = Modifier.height(200.dp)) {
                item {
                    TextButton(onClick = { onSelect(null) }, modifier = Modifier.fillMaxWidth()) {
                        CustomText("None")
                    }
                }
                items(Buttons.entries) { button ->
                    TextButton(onClick = { onSelect(button) }, modifier = Modifier.fillMaxWidth()) {
                        CustomText(button.name)
                    }
                }
            }
        },
        confirmButton = {},
        dismissButton = {
            TextButton(onClick = onDismiss) { CustomText("Cancel") }
        },
        containerColor = GameBoyColors.DarkGreen,
        shape = RectangleShape
    )
}

@Composable
private fun PalettePreview(colors: List<String>) {
    Row(horizontalArrangement = Arrangement.spacedBy(4.dp)) {
        colors.forEach { colorHex ->
            Box(
                modifier = Modifier
                    .size(16.dp)
                    .background(Color(colorHex.toColorInt()))
            )
        }
    }
}

private fun getAxisName(axis: Int): String {
  return when (axis) {
    MotionEvent.AXIS_X -> "X (Left Stick H)"
    MotionEvent.AXIS_Y -> "Y (Left Stick V)"
    MotionEvent.AXIS_Z -> "Z (Right Stick H)"
    MotionEvent.AXIS_RZ -> "RZ (Right Stick V)"
    MotionEvent.AXIS_HAT_X -> "HAT_X (D-Pad H)"
    MotionEvent.AXIS_HAT_Y -> "HAT_Y (D-Pad V)"
    MotionEvent.AXIS_LTRIGGER -> "L-Trigger"
    MotionEvent.AXIS_RTRIGGER -> "R-Trigger"
    else -> "Axis $axis"
  }
}
