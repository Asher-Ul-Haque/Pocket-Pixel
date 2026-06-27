package just.somebody.templates.appModule

import android.content.Context
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.ui.res.stringResource
import just.somebody.templates.R
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

/**
 * The state of the connected game controller
 * @property isConnected (Boolean) : whether the controller is connected
 * @property deviceName (String) : the name of the controller
 * @property buttons (Map) : a map of which buttons are pressed
 * @property axes (Map) : the axes of the controller and how much is their value
 */
data class GameControllerState(
  val isConnected : Boolean           = false,
  val deviceName  : String            = "No Controller",
  val buttons     : Map<Int, Boolean> = emptyMap(),
  val axes        : Map<Int, Float>   = emptyMap())

/**
 * The manager for game controller, which handles state and responds to hardware changes
 * @param controllerState : the current state of the controller
 */
interface GameControllerManager
{
  val controllerState: StateFlow<GameControllerState>

  /**
   * Handler for key presses on controller
   * @param EVENT (KeyEvent) : key press hardware event
   * @return (boolean) : whether the given key was pressed or not
   */
  fun handleKeyEvent(EVENT: KeyEvent): Boolean

  /**
   * Handler for controller axes changes
   * @param EVENT (MotionEvent) : axis change hardware event
   * @return (Boolean) : whether the axis changed
   */
  fun handleMotionEvent(EVENT: MotionEvent): Boolean

  /** Updates state */
  fun updateConnectionState()
}

/**
 * Standard Controller Manager implementation
 */
class DefaultGameControllerManager(private val CONTEXT: Context) : GameControllerManager
{
  private val _controllerState = MutableStateFlow(GameControllerState())
  override val controllerState = _controllerState.asStateFlow()

  init { updateConnectionState() }

  override fun updateConnectionState()
  {
    val deviceIds = InputDevice.getDeviceIds()
    var connected = false
    var name      = "No Controller"

    for (deviceId in deviceIds)
    {
      val device  = InputDevice.getDevice(deviceId) ?: continue
      val sources = device.sources
      if (((sources and InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
          ((sources and InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
        connected = true
        name      = device.name
        break
      }
    }

    _controllerState.update { it.copy(isConnected = connected, deviceName = name) }
  }

  override fun handleKeyEvent(EVENT: KeyEvent): Boolean
  {
    if (!isControllerEvent(EVENT)) return false

    val keyCode   = EVENT.keyCode
    val isPressed = EVENT.action == KeyEvent.ACTION_DOWN

    _controllerState.update()
    { state -> state.copy(buttons = state.buttons + (keyCode to isPressed)) }
    return true
  }

  override fun handleMotionEvent(EVENT: MotionEvent): Boolean
  {
    if (!isControllerEvent(EVENT)) return false

    val device  = EVENT.device ?: return false
    val newAxes = mutableMapOf<Int, Float>()

    // - - - Standard axes for gamepads
    val axesToTrack = intArrayOf(
      MotionEvent.AXIS_X,         MotionEvent.AXIS_Y,
      MotionEvent.AXIS_Z,         MotionEvent.AXIS_RZ,
      MotionEvent.AXIS_HAT_X,     MotionEvent.AXIS_HAT_Y,
      MotionEvent.AXIS_LTRIGGER,  MotionEvent.AXIS_RTRIGGER)

    for (axis in axesToTrack)
    {
      val range = device.getMotionRange(axis, EVENT.source) ?: continue
      val value = EVENT.getAxisValue(axis)
      if (Math.abs(value) > range.flat) newAxes[axis] = value
      else                              newAxes[axis] = 0f
    }

    _controllerState.update { state -> state.copy(axes = state.axes + newAxes) }
    return true
  }

  private fun isControllerEvent(EVENT: android.view.InputEvent): Boolean
  {
    val source = EVENT.source
    return ((source and InputDevice.SOURCE_GAMEPAD)   == InputDevice.SOURCE_GAMEPAD) ||
           ((source and InputDevice.SOURCE_JOYSTICK)  == InputDevice.SOURCE_JOYSTICK) ||
           ((source and InputDevice.SOURCE_DPAD)      == InputDevice.SOURCE_DPAD)
  }
}
