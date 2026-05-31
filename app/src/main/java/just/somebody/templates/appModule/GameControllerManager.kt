package just.somebody.templates.appModule

import android.content.Context
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

data class GameControllerState(
  val isConnected : Boolean = false,
  val deviceName  : String = "No Controller",
  val buttons     : Map<Int, Boolean> = emptyMap(),
  val axes        : Map<Int, Float> = emptyMap()
)

interface GameControllerManager
{
  val controllerState: StateFlow<GameControllerState>
  fun handleKeyEvent(EVENT: KeyEvent): Boolean
  fun handleMotionEvent(EVENT: MotionEvent): Boolean
  fun updateConnectionState()
}

class DefaultGameControllerManager(private val context: Context) : GameControllerManager
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
    
    _controllerState.update { state -> state.copy(buttons = state.buttons + (keyCode to isPressed)) }
    return true
  }

  override fun handleMotionEvent(EVENT: MotionEvent): Boolean
  {
    if (!isControllerEvent(EVENT)) return false
    
    val device  = EVENT.device ?: return false
    val newAxes = mutableMapOf<Int, Float>()
    
    // - - - Standard axes for gamepads
    val axesToTrack = intArrayOf(
      MotionEvent.AXIS_X, MotionEvent.AXIS_Y,
      MotionEvent.AXIS_Z, MotionEvent.AXIS_RZ,
      MotionEvent.AXIS_HAT_X, MotionEvent.AXIS_HAT_Y,
      MotionEvent.AXIS_LTRIGGER, MotionEvent.AXIS_RTRIGGER
    )

    for (axis in axesToTrack)
    {
      val range = device.getMotionRange(axis, EVENT.source) ?: continue
      val value = EVENT.getAxisValue(axis)
      if (Math.abs(value) > range.flat)
      {
        newAxes[axis] = value
      }
      else
      {
        newAxes[axis] = 0f
      }
    }

    _controllerState.update { state -> state.copy(axes = state.axes + newAxes) }
    return true
  }

  private fun isControllerEvent(EVENT: android.view.InputEvent): Boolean
  {
    val source = EVENT.source
    return ((source and InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
           ((source and InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK) ||
           ((source and InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD)
  }
}
