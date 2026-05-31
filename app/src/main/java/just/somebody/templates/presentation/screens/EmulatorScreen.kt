package just.somebody.templates.presentation.screens

import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import just.somebody.templates.App
import just.somebody.templates.domain.Buttons
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.presentation.widgets.GameBoyFrame
import kotlin.math.abs

@Composable
fun EmulatorScreen(
  MODIFIER    : Modifier = Modifier,
  VIEW_MODEL  : EmulatorViewModel,
  URI         : String
)
{
  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val gameBoy = App.appModule.gameBoy

  // Map Gamepad Buttons to GameBoy Buttons
  LaunchedEffect(controllerState.buttons) {
    val buttons = controllerState.buttons
    gameBoy.sendButton(Buttons.A, buttons[KeyEvent.KEYCODE_BUTTON_A] == true)
    gameBoy.sendButton(Buttons.B, buttons[KeyEvent.KEYCODE_BUTTON_B] == true || buttons[KeyEvent.KEYCODE_BUTTON_X] == true)
    gameBoy.sendButton(Buttons.START, buttons[KeyEvent.KEYCODE_BUTTON_START] == true)
    gameBoy.sendButton(Buttons.SELECT, buttons[KeyEvent.KEYCODE_BUTTON_SELECT] == true)
    
    // D-Pad key fallback
    if (buttons.containsKey(KeyEvent.KEYCODE_DPAD_UP)) gameBoy.sendButton(Buttons.UP, buttons[KeyEvent.KEYCODE_DPAD_UP] == true)
    if (buttons.containsKey(KeyEvent.KEYCODE_DPAD_DOWN)) gameBoy.sendButton(Buttons.DOWN, buttons[KeyEvent.KEYCODE_DPAD_DOWN] == true)
    if (buttons.containsKey(KeyEvent.KEYCODE_DPAD_LEFT)) gameBoy.sendButton(Buttons.LEFT, buttons[KeyEvent.KEYCODE_DPAD_LEFT] == true)
    if (buttons.containsKey(KeyEvent.KEYCODE_DPAD_RIGHT)) gameBoy.sendButton(Buttons.RIGHT, buttons[KeyEvent.KEYCODE_DPAD_RIGHT] == true)
  }

  // Map Analog Sticks / Hat to GameBoy D-Pad
  LaunchedEffect(controllerState.axes) {
    val axes = controllerState.axes
    val hatX = axes[MotionEvent.AXIS_HAT_X] ?: 0f
    val hatY = axes[MotionEvent.AXIS_HAT_Y] ?: 0f
    val stickX = axes[MotionEvent.AXIS_X] ?: 0f
    val stickY = axes[MotionEvent.AXIS_Y] ?: 0f

    val threshold = 0.5f
    
    val up = hatY < -threshold || stickY < -threshold
    val down = hatY > threshold || stickY > threshold
    val left = hatX < -threshold || stickX < -threshold
    val right = hatX > threshold || stickX > threshold

    gameBoy.sendButton(Buttons.UP, up)
    gameBoy.sendButton(Buttons.DOWN, down)
    gameBoy.sendButton(Buttons.LEFT, left)
    gameBoy.sendButton(Buttons.RIGHT, right)
  }

  DisposableEffect(Unit)
  {
    onDispose { VIEW_MODEL.stopEmulator() }
  }

  Column(
    modifier            = MODIFIER
      .fillMaxSize()
      .background(Color.Black)
      .padding(top = 48.dp),
    verticalArrangement = Arrangement.Bottom,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    val gameBoyAspectRatio = 160f / 144f

    AndroidView(
      modifier  = Modifier
        .fillMaxWidth()
        .aspectRatio(gameBoyAspectRatio),
      factory   =
        { context ->
          val gameBoySurfaceView = GameBoyFrame(context)
          gameBoySurfaceView
        },
      update    = { }
    )

    Spacer(modifier = Modifier.height(48.dp))

    GameBoyControls(App.appModule.gameBoy, VIEW_MODEL)
  }
}