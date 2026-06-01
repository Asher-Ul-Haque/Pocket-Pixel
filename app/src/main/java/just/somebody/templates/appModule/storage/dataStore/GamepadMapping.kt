package just.somebody.templates.appModule.storage.dataStore

import android.view.KeyEvent
import android.view.MotionEvent
import just.somebody.templates.domain.Buttons
import kotlinx.serialization.Serializable

/**
 * Data architecture component defining hardware input translation matrices.
 *
 * Maps physical controller signals (discrete button presses and continuous axis motions)
 * down to their associated target emulation keys. This structure is safely serialized
 * inside data storage to maintain custom user controller maps across runtime iterations.
 *
 * @property buttonToGameBoy Key-value map indexing Android digital [KeyEvent] codes to specific emulator buttons.
 * @property axisToGameBoy Nested lookup map assigning analog directional thresholds (e.g., thumbsticks or D-pad hats)
 * to target emulator inputs based on the direction of their raw axis coordinate values (-1 or 1).
 * @property deadzone A normalized threshold boundary (0.0f to 1.0f) below which analog stick input fluctuations
 * are ignored to prevent accidental drift processing.
 */
@Serializable
data class GamepadMapping(
  val buttonToGameBoy: Map<Int, Buttons> = mapOf(
    KeyEvent.KEYCODE_BUTTON_A       to Buttons.A,
    KeyEvent.KEYCODE_BUTTON_B       to Buttons.B,
    KeyEvent.KEYCODE_BUTTON_X       to Buttons.B,
    KeyEvent.KEYCODE_BUTTON_START   to Buttons.START,
    KeyEvent.KEYCODE_BUTTON_SELECT  to Buttons.SELECT,
    KeyEvent.KEYCODE_DPAD_UP        to Buttons.UP,
    KeyEvent.KEYCODE_DPAD_DOWN      to Buttons.DOWN,
    KeyEvent.KEYCODE_DPAD_LEFT      to Buttons.LEFT,
    KeyEvent.KEYCODE_DPAD_RIGHT     to Buttons.RIGHT
                                                ),
  val axisToGameBoy: Map<Int, Map<Int, Buttons>> = mapOf(
    MotionEvent.AXIS_X      to mapOf(-1 to Buttons.LEFT, 1  to Buttons.RIGHT),
    MotionEvent.AXIS_Y      to mapOf(-1 to Buttons.UP, 1    to Buttons.DOWN),
    MotionEvent.AXIS_HAT_X  to mapOf(-1 to Buttons.LEFT, 1  to Buttons.RIGHT),
    MotionEvent.AXIS_HAT_Y  to mapOf(-1 to Buttons.UP, 1    to Buttons.DOWN)),
  val deadzone: Float = 0.2f)