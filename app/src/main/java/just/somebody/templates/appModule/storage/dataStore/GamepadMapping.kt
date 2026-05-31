package just.somebody.templates.appModule.storage.dataStore

import android.view.KeyEvent
import android.view.MotionEvent
import just.somebody.templates.domain.Buttons
import kotlinx.serialization.Serializable

@Serializable
data class GamepadMapping(
    val buttonToGameBoy: Map<Int, Buttons> = mapOf(
        KeyEvent.KEYCODE_BUTTON_A to Buttons.A,
        KeyEvent.KEYCODE_BUTTON_B to Buttons.B,
        KeyEvent.KEYCODE_BUTTON_X to Buttons.B,
        KeyEvent.KEYCODE_BUTTON_START to Buttons.START,
        KeyEvent.KEYCODE_BUTTON_SELECT to Buttons.SELECT,
        KeyEvent.KEYCODE_DPAD_UP to Buttons.UP,
        KeyEvent.KEYCODE_DPAD_DOWN to Buttons.DOWN,
        KeyEvent.KEYCODE_DPAD_LEFT to Buttons.LEFT,
        KeyEvent.KEYCODE_DPAD_RIGHT to Buttons.RIGHT
    ),
    val axisToGameBoy: Map<Int, Map<Int, Buttons>> = mapOf(
        MotionEvent.AXIS_X to mapOf(-1 to Buttons.LEFT, 1 to Buttons.RIGHT),
        MotionEvent.AXIS_Y to mapOf(-1 to Buttons.UP, 1 to Buttons.DOWN),
        MotionEvent.AXIS_HAT_X to mapOf(-1 to Buttons.LEFT, 1 to Buttons.RIGHT),
        MotionEvent.AXIS_HAT_Y to mapOf(-1 to Buttons.UP, 1 to Buttons.DOWN)
    ),
    val deadzone: Float = 0.2f
)
