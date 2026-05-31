package just.somebody.templates.presentation.screens

import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.PauseTrigger
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyActionButtons
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.presentation.widgets.GameBoyDpad
import just.somebody.templates.presentation.widgets.GameBoyFrame
import just.somebody.templates.presentation.widgets.NormalButton
import just.somebody.templates.presentation.widgets.SettingsPanel
import just.somebody.templates.ui.theme.GameBoyColors

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EmulatorScreen(
  MODIFIER    : Modifier = Modifier,
  VIEW_MODEL  : EmulatorViewModel,
  URI         : String
)
{
  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val settings        by VIEW_MODEL.settings.collectAsState()
  val gameBoy       = App.appModule.gameBoy
  val showSettings  = remember { mutableStateOf(false) }

  // - - - Handle App Focus/Lifecycle Pause
  val lifecycleOwner = LocalLifecycleOwner.current
  DisposableEffect(lifecycleOwner) {
    val observer = LifecycleEventObserver { _, event ->
      when (event) {
        Lifecycle.Event.ON_PAUSE -> VIEW_MODEL.pause(PauseTrigger.FOCUS)
        Lifecycle.Event.ON_RESUME -> VIEW_MODEL.resume(PauseTrigger.FOCUS)
        else -> {}
      }
    }
    lifecycleOwner.lifecycle.addObserver(observer)
    onDispose {
      lifecycleOwner.lifecycle.removeObserver(observer)
    }
  }

  // - - - Handle Settings Modal Pause
  LaunchedEffect(showSettings.value) {
    if (showSettings.value) VIEW_MODEL.pause(PauseTrigger.SETTINGS)
    else VIEW_MODEL.resume(PauseTrigger.SETTINGS)
  }

  // - - - Map Gamepad Buttons to GameBoy Buttons based on custom mapping
  LaunchedEffect(controllerState.buttons, settings.gamepadMapping)
  {
    val buttons = controllerState.buttons
    val mapping = settings.gamepadMapping.buttonToGameBoy
    
    // - - - Clear states for buttons that are in our mapping
    mapping.values.distinct().forEach()
    { gbButton ->
      val isPressed = mapping.filter { it.value == gbButton }.any()
      { (keyCode, _) -> buttons[keyCode] == true }
      gameBoy.sendButton(gbButton, isPressed)
    }
  }

  // Map Analog Sticks / Hat to GameBoy D-Pad based on custom mapping
  LaunchedEffect(controllerState.axes, settings.gamepadMapping) {
    val axes = controllerState.axes
    val mapping = settings.gamepadMapping.axisToGameBoy
    val deadzone = settings.gamepadMapping.deadzone
    
    // Apply custom mapping
    val allGbButtons = mapping.values.flatMap { it.values }.distinct()
    allGbButtons.forEach { gbButton ->
        val isPressed = mapping.any { (axis, dirMap) ->
            dirMap.any { (dir, mappedButton) ->
                if (mappedButton == gbButton) {
                    val value = axes[axis] ?: 0f
                    if (dir > 0) value > deadzone else value < -deadzone
                } else false
            }
        }
        gameBoy.sendButton(gbButton, isPressed)
    }
  }

  DisposableEffect(Unit)
  {
    onDispose { VIEW_MODEL.stopEmulator() }
  }

  val isLandscape = App.appModule.isLandscape()
  val gameBoyAspectRatio = 160f / 144f

  Box(
    modifier = MODIFIER
      .fillMaxSize()
      .background(Color.Black)
  ) {
    if (isLandscape) {
      Box(modifier = Modifier.fillMaxSize().background(GameBoyColors.DarkGreen)) {
        Row(
          modifier = Modifier.fillMaxSize(),
          verticalAlignment = Alignment.CenterVertically,
          horizontalArrangement = Arrangement.SpaceBetween
        ) {
          // Left Controls: Dpad and Select
          Column(
            modifier = Modifier.padding(start = 16.dp, end = 8.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
          ) {
            GameBoyDpad(gameBoy)
            Spacer(modifier = Modifier.height(24.dp))
            NormalButton("Select", Buttons.SELECT, gameBoy)
          }

          // Viewport
          Box(
            modifier = Modifier.weight(1f),
            contentAlignment = Alignment.Center
          ) {
            AndroidView(
              modifier = Modifier
                .fillMaxHeight()
                .aspectRatio(gameBoyAspectRatio),
              factory = { context -> GameBoyFrame(context) },
              update = { }
            )
          }

          // Right Controls: Actions and Start
          Column(
            modifier = Modifier.padding(start = 8.dp, end = 16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
          ) {
            GameBoyActionButtons(gameBoy)
            Spacer(modifier = Modifier.height(24.dp))
            NormalButton("Start", Buttons.START, gameBoy)
          }
        }

        // Settings icon for landscape - Top Right
        Icon(
          painter             = painterResource(R.drawable.settings),
          contentDescription  = "In-game Settings",
          tint                = GameBoyColors.MediumGreen,
          modifier            = Modifier
            .align(Alignment.TopEnd)
            .padding(16.dp)
            .size(32.dp)
            .clickable { showSettings.value = true }
        )
      }
    } else {
      Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally
      ) {
        // Viewport (fills top, but maintains ratio)
        Box(
          modifier = Modifier
            .weight(1f)
            .fillMaxWidth()
            .background(Color.Black),
          contentAlignment = Alignment.Center)
        {
          AndroidView(
            modifier = Modifier
              .fillMaxWidth(1.0f)
              .aspectRatio(gameBoyAspectRatio),
            factory = { context -> GameBoyFrame(context) },
            update  = { }
          )
        }

        // - - - Controls at bottom
        GameBoyControls(gameBoy, VIEW_MODEL) { showSettings.value = true }
      }
    }

    if (showSettings.value)
    {
      SettingsPanel(
        GAME_BOY = gameBoy,
        EMULATOR = VIEW_MODEL,
        ON_CLOSE = { showSettings.value = false })
    }
  }
}
