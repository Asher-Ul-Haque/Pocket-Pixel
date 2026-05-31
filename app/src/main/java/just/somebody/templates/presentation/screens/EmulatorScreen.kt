package just.somebody.templates.presentation.screens

import android.view.KeyEvent
import android.view.MotionEvent
import androidx.compose.animation.core.animateDpAsState
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.input.pointer.PointerEventPass
import androidx.compose.ui.input.pointer.pointerInput
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
import kotlinx.coroutines.delay

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

  // Immersive Mode Logic: Fade out controls after 5 seconds of inactivity
  var lastInteractionTime by remember { mutableStateOf(System.currentTimeMillis()) }
  var controlsVisible by remember { mutableStateOf(true) }
  
  val controlAlpha by animateFloatAsState(
      targetValue = if (!settings.isImmersiveModeEnabled || controlsVisible || showSettings.value) 1f else 0f,
      animationSpec = tween(durationMillis = 1000),
      label = "controlAlpha"
  )

  LaunchedEffect(lastInteractionTime, showSettings.value, settings.isImmersiveModeEnabled) {
      if (!showSettings.value && settings.isImmersiveModeEnabled) {
          delay(5000)
          controlsVisible = false
      }
  }

  val onInteraction = {
      lastInteractionTime = System.currentTimeMillis()
      controlsVisible = true
  }

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

  // Viewport slide animation
  // In Portrait: Center when UI hidden (0dp), Top when UI visible (-120dp).
  val viewportYOffset by animateDpAsState(
      targetValue = if (!isLandscape && (controlsVisible || showSettings.value)) (-120).dp else 0.dp,
      animationSpec = tween(durationMillis = 1000),
      label = "viewportSlide"
  )

  BoxWithConstraints(
    modifier = MODIFIER
      .fillMaxSize()
      .background(Color.Black)
      .pointerInput(Unit) {
          // Reset timer on any touch, even if consumed by children
          awaitPointerEventScope {
              while (true) {
                  awaitPointerEvent(PointerEventPass.Initial)
                  onInteraction()
              }
          }
      }
  ) {
    val screenWidth = maxWidth
    val screenHeight = maxHeight
    val viewportActualWidth = if (isLandscape) (screenHeight * gameBoyAspectRatio) else screenWidth

    // 1. Viewport Layer (Always visible, centered by default)
    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
        AndroidView(
          modifier = Modifier
            .fillMaxHeight(if (isLandscape) 1f else 0.7f)
            .aspectRatio(gameBoyAspectRatio)
            .offset(y = viewportYOffset),
          factory = { context -> GameBoyFrame(context) },
          update = { }
        )
    }

    // 2. Controls Layer
    if (isLandscape) {
      Row(modifier = Modifier.fillMaxSize()) {
        // Left Side Panel
        Box(modifier = Modifier
            .fillMaxHeight()
            .weight(1f)
            .alpha(controlAlpha)
            .background(GameBoyColors.DarkGreen)
            .padding(8.dp),
            contentAlignment = Alignment.Center
        ) {
          Column(horizontalAlignment = Alignment.CenterHorizontally, modifier = Modifier.width(IntrinsicSize.Min)) {
            GameBoyDpad(gameBoy)
            Spacer(modifier = Modifier.height(32.dp))
            NormalButton("Select", Buttons.SELECT, gameBoy)
          }
        }

        // Viewport Gap (Ensures side panels don't overlap centered game area)
        Spacer(modifier = Modifier.width(viewportActualWidth))

        // Right Side Panel
        Box(modifier = Modifier
            .fillMaxHeight()
            .weight(1f)
            .alpha(controlAlpha)
            .background(GameBoyColors.DarkGreen)
            .padding(8.dp),
            contentAlignment = Alignment.Center
        ) {
          Column(horizontalAlignment = Alignment.CenterHorizontally, modifier = Modifier.width(IntrinsicSize.Min)) {
            GameBoyActionButtons(gameBoy)
            Spacer(modifier = Modifier.height(32.dp))
            NormalButton("Start", Buttons.START, gameBoy)
          }
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
          .alpha(controlAlpha)
          .clickable { 
              onInteraction()
              showSettings.value = true 
          }
      )
    } else {
      // Portrait Mode
      Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Bottom
      ) {
        // Portrait Controls Layer - Fades to reveal Black root
        Box(modifier = Modifier
            .fillMaxWidth()
            .alpha(controlAlpha)
            .background(GameBoyColors.DarkGreen)
        ) {
            GameBoyControls(gameBoy, VIEW_MODEL, onInteraction) { 
                onInteraction()
                showSettings.value = true 
            }
        }
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
