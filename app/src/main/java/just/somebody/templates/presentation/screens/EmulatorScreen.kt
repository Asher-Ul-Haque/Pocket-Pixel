package just.somebody.templates.presentation.screens

import android.annotation.SuppressLint
import androidx.compose.animation.core.animateDpAsState
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
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
import just.somebody.templates.ui.theme.DeviceSizePreviews
import kotlinx.coroutines.delay

/**
 * Core runtime terminal shell managing active game simulation layouts and state sync loops.
 *
 * This wrapper interceptor orchestrates critical lifecycle callbacks (such as hardware focus
 * losses or layout setting disruptions), maps incoming external game controller axis signals
 * through custom user-defined deadzones, and handles responsive alpha fade configurations
 * to implement an automatic immersive screen mode during prolonged input inactivity.
 *
 * @param MODIFIER [Modifier] used to establish positional layout bounds or boundary dimensions.
 * @param VIEW_MODEL State supervisor tracking global preference parameters and emulator runtime controls.
 * @param URI The explicit string data address location pointing straight to the target game ROM file.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EmulatorScreen(
  MODIFIER    : Modifier = Modifier,
  VIEW_MODEL  : EmulatorViewModel,
  URI         : String)
{
  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  val controllerState by App.appModule.gameControllerManager.controllerState.collectAsState()
  val settings        by VIEW_MODEL.settings.collectAsState()
  val gameBoy       = App.appModule.gameBoy
  val showSettings  = remember { mutableStateOf(false) }

  // - - - Immersive Mode Logic: Fade out controls after 5 seconds of inactivity
  var lastInteractionTime by remember { mutableStateOf(System.currentTimeMillis()) }
  var controlsVisible     by remember { mutableStateOf(true) }

  val controlAlpha by animateFloatAsState(
    targetValue =
      if (!settings.isImmersiveModeEnabled || controlsVisible || showSettings.value)  1f
      else                                                                            0f,
    animationSpec = tween(durationMillis = 1000),
    label         = "controlAlpha")

  LaunchedEffect(lastInteractionTime, showSettings.value, settings.isImmersiveModeEnabled)
  {
    if (!showSettings.value && settings.isImmersiveModeEnabled)
    {
      delay(5000)
      controlsVisible = false
    }
  }

  val onInteraction =
    {
      lastInteractionTime = System.currentTimeMillis()
      controlsVisible     = true
    }

  // - - - Handle App Focus/Lifecycle Pause
  val lifecycleOwner = LocalLifecycleOwner.current
  DisposableEffect(lifecycleOwner)
  {
    val observer = LifecycleEventObserver { _, event ->
      when (event)
      {
        Lifecycle.Event.ON_PAUSE  -> VIEW_MODEL.pause(PauseTrigger.FOCUS)
        Lifecycle.Event.ON_RESUME -> VIEW_MODEL.resume(PauseTrigger.FOCUS)
        else                      -> {}
      }
    }
    lifecycleOwner.lifecycle.addObserver(observer)
    onDispose { lifecycleOwner.lifecycle.removeObserver(observer) }
  }

  // - - - Handle Settings Modal Pause
  LaunchedEffect(showSettings.value)
  {
    if (showSettings.value) VIEW_MODEL.pause(PauseTrigger.SETTINGS)
    else                    VIEW_MODEL.resume(PauseTrigger.SETTINGS)
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

  // - - - Map Analog Sticks / Hat to GameBoy D-Pad based on custom mapping
  LaunchedEffect(controllerState.axes, settings.gamepadMapping)
  {
    val axes      = controllerState.axes
    val mapping   = settings.gamepadMapping.axisToGameBoy
    val deadzone  = settings.gamepadMapping.deadzone

    // - - - Apply custom mapping
    val allGbButtons = mapping.values.flatMap { it.values }.distinct()
    allGbButtons.forEach()
    { gbButton ->
      val isPressed = mapping.any()
      { (axis, dirMap) ->
        dirMap.any()
        { (dir, mappedButton) ->
          if (mappedButton == gbButton)
          {
            val value = axes[axis] ?: 0f
            if (dir > 0)  value > deadzone
            else          value < -deadzone
          }
          else false
        }
      }
      gameBoy.sendButton(gbButton, isPressed)
    }
  }

  DisposableEffect(Unit)
  {
    onDispose { VIEW_MODEL.stopEmulator() }
  }

  val isLandscape         = App.appModule.isLandscape()
  val gameBoyAspectRatio  = 160f / 144f

  EmulatorContent(
    IS_LANDSCAPE       = isLandscape,
    CONTROLS_VISIBLE   = controlsVisible,
    CONTROL_ALPHA      = controlAlpha,
    SHOW_SETTINGS      = showSettings.value,
    ON_INTERACTION     = onInteraction,
    ON_TOGGLE_SETTINGS  = { showSettings.value = !showSettings.value },
    VIEWPORT          = { modifier ->
      AndroidView(
        modifier  = modifier,
        factory   = { context -> GameBoyFrame(context) },
        update    = { })
    },
    CONTROLS =
      {
        GameBoyControls(gameBoy, VIEW_MODEL, onInteraction)
        {
          onInteraction()
          showSettings.value = true
        }
      },
    SETTINGS_PANEL =
      {
        if (showSettings.value)
        {
          SettingsPanel(
            GAME_BOY = gameBoy,
            EMULATOR = VIEW_MODEL,
            ON_CLOSE = { showSettings.value = false })
        }
      },
    D_PAD          = { GameBoyDpad(gameBoy) },
    ACTION_BUTTONS = { GameBoyActionButtons(gameBoy) },
    SELECT_BUTTON  = { NormalButton("Select", Buttons.SELECT, gameBoy) },
    START_BUTTON   = { NormalButton("Start", Buttons.START, gameBoy) },
    MODIFIER      = MODIFIER)
}

/**
 * Presentation sandbox component rendering split interface styles depending on screen angles.
 *
 * Places the hardware rendering viewport on a distinct alignment track that slides dynamically,
 * positioning cross-pads and button slots along a bottom pane block during portrait configurations,
 * or dividing them into flanking side wings when the parent architecture signals a landscape rotation.
 *
 * @param IS_LANDSCAPE Target device positioning flag used to pivot container layout directions.
 * @param CONTROLS_VISIBLE Telemetry status signaling if interface buttons are currently rendering.
 * @param CONTROL_ALPHA Continuous decimal scale bound to fading animation transparency curves.
 * @param SHOW_SETTINGS Control variable indicating if configuration overrides are active.
 * @param ON_INTERACTION Callback listener intercepting tap coordinates across any layer slot.
 * @param ON_TOGGLE_SETTINGS State mutation dispatch swapping option overlay triggers.
 * @param VIEWPORT Composable layout capsule passing down the low-level rendering surface framework view.
 * @param CONTROLS Layout builder composing portrait dashboard input clusters.
 * @param SETTINGS_PANEL Structural layer holding real-time overlay adjustment tabs.
 * @param D_PAD Directional directional cross sub-component slot.
 * @param ACTION_BUTTONS Primary execution navigation key sub-component slot.
 * @param SELECT_BUTTON Tactile choice switch controller node.
 * @param START_BUTTON Tactile initialization cycle controller node.
 * @param ASPECT_RATIO Proportional layout fraction locking display windows to historical bounds.
 * @param MODIFIER [Modifier] used to arrange outer container boundaries.
 */
@SuppressLint("UnusedBoxWithConstraintsScope")
@Composable
fun EmulatorContent(
  IS_LANDSCAPE      : Boolean,
  CONTROLS_VISIBLE  : Boolean,
  CONTROL_ALPHA     : Float,
  SHOW_SETTINGS     : Boolean,
  ON_INTERACTION    : () -> Unit,
  ON_TOGGLE_SETTINGS: () -> Unit,
  VIEWPORT          : @Composable (Modifier) -> Unit,
  CONTROLS          : @Composable () -> Unit,
  SETTINGS_PANEL    : @Composable () -> Unit,
  D_PAD             : @Composable () -> Unit,
  ACTION_BUTTONS    : @Composable () -> Unit,
  SELECT_BUTTON     : @Composable () -> Unit,
  START_BUTTON      : @Composable () -> Unit,
  ASPECT_RATIO      : Float = 160f / 144f,
  MODIFIER          : Modifier = Modifier)
{
  // - - - Viewport slide animation
  val viewportYOffset by animateDpAsState(
    targetValue =
      if (!IS_LANDSCAPE && (CONTROLS_VISIBLE || SHOW_SETTINGS)) (-120).dp
      else                                                      0.dp,
    animationSpec = tween(durationMillis = 1000),
    label         = "viewportSlide")

  BoxWithConstraints(
    modifier = MODIFIER
      .fillMaxSize()
      .background(Color.Black)
      .pointerInput(Unit)
      {
        awaitPointerEventScope()
        {
          while (true)
          {
            awaitPointerEvent(PointerEventPass.Initial)
            ON_INTERACTION()
          }
        }
      })
  {
    val screenHeight        = maxHeight
    val screenWidth         = maxWidth
    val viewportActualWidth =
      if (IS_LANDSCAPE) (screenHeight * ASPECT_RATIO)
      else              screenWidth

    // - - - 1. Viewport Layer (Always visible, centered by default)
    Box(
      modifier          = Modifier.fillMaxSize(),
      contentAlignment  = Alignment.Center)
    {
      VIEWPORT(
        Modifier
          .fillMaxHeight(
            if (IS_LANDSCAPE) 1f
            else              0.7f)
          .aspectRatio(ASPECT_RATIO)
          .offset(y = viewportYOffset))
    }

    // - - - 2. Controls Layer
    if (IS_LANDSCAPE)
    {
      Row(modifier = Modifier.fillMaxSize())
      {
        // - - - Left Side Panel
        Box(modifier = Modifier
          .fillMaxHeight()
          .weight(1f)
          .alpha(CONTROL_ALPHA)
          .background(GameBoyColors.DarkGreen)
          .padding(8.dp),
            contentAlignment = Alignment.Center)
        {
          Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier            = Modifier.width(IntrinsicSize.Min))
          {
            D_PAD()
            Spacer(modifier = Modifier.height(32.dp))
            SELECT_BUTTON()
          }
        }

        // - - - Viewport Gap
        Spacer(modifier = Modifier.width(viewportActualWidth))

        // - - - Right Side Panel
        Box(modifier = Modifier
          .fillMaxHeight()
          .weight(1f)
          .alpha(CONTROL_ALPHA)
          .background(GameBoyColors.DarkGreen)
          .padding(8.dp),
            contentAlignment = Alignment.Center)
        {
          Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier            = Modifier.width(IntrinsicSize.Min))
          {
            ACTION_BUTTONS()
            Spacer(modifier = Modifier.height(32.dp))
            START_BUTTON()
          }
        }
      }

      // - - - Settings icon for landscape - Top Right
      Icon(
        painter             = painterResource(R.drawable.settings),
        contentDescription  = null,
        tint                = GameBoyColors.MediumGreen,
        modifier            = Modifier
          .align(Alignment.TopEnd)
          .padding(16.dp)
          .size(32.dp)
          .alpha(CONTROL_ALPHA)
          .clickable
          {
            ON_INTERACTION()
            ON_TOGGLE_SETTINGS()
          })
    }
    else
    {
      // - - - Portrait Mode
      Column(
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Bottom)
      {
        // - - - Portrait Controls Layer
        Box(modifier = Modifier
          .fillMaxWidth()
          .alpha(CONTROL_ALPHA)
          .background(GameBoyColors.DarkGreen))
        { CONTROLS() }
      }
    }

    SETTINGS_PANEL()
  }
}

/**
 * Design-time layout preview reconstructing a vertical portrait console alignment profile.
 */
@DeviceSizePreviews
@Composable
private fun EmulatorPreviewPortrait()
{
  EmulatorContent(
    IS_LANDSCAPE        = false,
    CONTROLS_VISIBLE    = true,
    CONTROL_ALPHA       = 1f,
    SHOW_SETTINGS       = false,
    ON_INTERACTION      = {},
    ON_TOGGLE_SETTINGS  = {},
    VIEWPORT            = { Box(modifier = it.background(Color.DarkGray)) },
    CONTROLS            =
      {
        Box(
          modifier = Modifier
            .fillMaxWidth()
            .height(250.dp)
            .background(GameBoyColors.DarkGreen))
      },
    SETTINGS_PANEL    = {},
    D_PAD             =
      {
        Box(
          modifier = Modifier
            .size(120.dp)
            .background(Color.Gray))
      },
    ACTION_BUTTONS    =
      {
        Box(
          modifier = Modifier
            .size(120.dp)
            .background(Color.Gray))
      },
    SELECT_BUTTON     =
      {
        Box(
          modifier = Modifier
            .size(80.dp, 40.dp)
            .background(Color.Gray))
      },
    START_BUTTON      =
      {
        Box(
          modifier = Modifier
            .size(80.dp, 40.dp)
            .background(Color.Gray))
      })
}

/**
 * Design-time layout preview reconstructing a wide landscape layout input split profile.
 */
@DeviceSizePreviews
@Composable
private fun EmulatorPreviewLandscape()
{
  EmulatorContent(
    IS_LANDSCAPE        = true,
    CONTROLS_VISIBLE    = true,
    CONTROL_ALPHA       = 1f,
    SHOW_SETTINGS       = false,
    ON_INTERACTION      = {},
    ON_TOGGLE_SETTINGS  = {},
    VIEWPORT            = { Box(modifier = it.background(Color.DarkGray)) },
    CONTROLS            = {},
    SETTINGS_PANEL      = {},
    D_PAD               =
      {
        Box(
          modifier = Modifier
            .size(120.dp)
            .background(Color.Gray))
      },
    ACTION_BUTTONS      =
      {
        Box(
          modifier = Modifier
            .size(120.dp)
            .background(Color.Gray))
      },
    SELECT_BUTTON =
      {
        Box(
          modifier = Modifier
            .size(80.dp, 40.dp)
            .background(Color.Gray))
      },
    START_BUTTON  =
      {
        Box(
          modifier = Modifier
            .size(80.dp, 40.dp)
            .background(Color.Gray))
      })
}