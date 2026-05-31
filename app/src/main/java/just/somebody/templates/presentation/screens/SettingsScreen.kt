package just.somebody.templates.presentation.screens

import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun SettingsScreen(
  VIEW_MODEL : SettingsViewModel,
  MODIFIFER  : Modifier = Modifier
)
{
  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val scope = rememberCoroutineScope()

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen)
    .padding(16.dp))
  {
    LazyColumn(
      modifier = Modifier.fillMaxSize(),
      verticalArrangement = Arrangement.spacedBy(8.dp),
      horizontalAlignment = Alignment.CenterHorizontally
    ) {
      item {
        Row(
          modifier = Modifier.fillMaxWidth(),
          horizontalArrangement = Arrangement.SpaceBetween,
          verticalAlignment = Alignment.CenterVertically
        ) {
          CustomText(
            TEXT = stringResource(R.string.SETTINGS),
            FONT_SIZE = 24,
            COLOR = GameBoyColors.LightGreen
          )
          IconButton(onClick = { VIEW_MODEL.updateControllerConnection() }) {
            Icon(
              painter = painterResource(R.drawable.settings), // Reusing settings icon for "refresh"
              contentDescription = "Refresh Controllers",
              tint = GameBoyColors.LightGreen
            )
          }
        }
      }

      item {
        Card(
          colors = CardDefaults.cardColors(containerColor = GameBoyColors.MediumGreen),
          modifier = Modifier.fillMaxWidth()
        ) {
          Column(modifier = Modifier.padding(16.dp)) {
            CustomText("Controller: ${controllerState.deviceName}")
            CustomText("Connected: ${controllerState.isConnected}")
          }
        }
      }

      if (controllerState.isConnected) {
        item {
          CustomText("Buttons Pressed:", FONT_SIZE = 18)
        }

        val pressedButtons = controllerState.buttons.filter { it.value }.keys.toList()
        if (pressedButtons.isEmpty()) {
          item { CustomText("None", COLOR = GameBoyColors.Green) }
        } else {
          items(pressedButtons) { keyCode ->
            CustomText("Key: ${KeyEvent.keyCodeToString(keyCode)}", COLOR = GameBoyColors.Green)
          }
        }

        item {
          CustomText("Axes Values:", FONT_SIZE = 18)
        }

        val activeAxes = controllerState.axes.filter { Math.abs(it.value) > 0.1f }
        if (activeAxes.isEmpty()) {
          item { CustomText("All Centered", COLOR = GameBoyColors.Green) }
        } else {
          items(activeAxes.toList()) { (axis, value) ->
             CustomText("Axis ${getAxisName(axis)}: ${"%.2f".format(value)}", COLOR = GameBoyColors.Green)
          }
        }
      }

      item {
        Spacer(modifier = Modifier.height(24.dp))
        CustomButton(
          ON_CLICK = { VIEW_MODEL.rescan() },
          MODIFIER = Modifier.fillMaxWidth()
        ) {
          CustomText(stringResource(R.string.RESCAN))
        }
      }

      item {
        CustomButton(
          ON_CLICK = { VIEW_MODEL.factoryReset() },
          MODIFIER = Modifier.fillMaxWidth(),
          COLOR = GameBoyColors.Error
        ) {
          CustomText(stringResource(R.string.FACTORY))
        }
      }
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
