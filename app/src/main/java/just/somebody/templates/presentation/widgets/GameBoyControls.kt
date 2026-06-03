package just.somebody.templates.presentation.widgets

import androidx.compose.animation.core.animateDpAsState
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.R
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily

/**
 * Renders the retro directional D-pad input layout grid.
 *
 * It combines explicit axis pads with invisible intersection corner buttons to interpret
 * diagonal micro-switches properly, feeding continuous state back to the emulator core.
 *
 * @param GAME_BOY The active [GameBoy] hardware wrapper engine receiving input updates.
 */
@Composable
fun GameBoyDpad(GAME_BOY: GameBoy)
{
  val lastDirection      : MutableState<Buttons?>     = remember { mutableStateOf<Buttons?>(null) }
  val activeDpadButtons  : MutableState<Set<Buttons>> = remember { mutableStateOf(setOf<Buttons>()) }

  Column(
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally)
  {
    Row(horizontalArrangement = Arrangement.Center)
    {
      DirectionButton(Buttons.UP, Buttons.LEFT,   GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.UP, null,           GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
      DirectionButton(Buttons.UP, Buttons.RIGHT,  GAME_BOY, lastDirection, activeDpadButtons)
    }
    Row(horizontalArrangement = Arrangement.Center)
    {
      DirectionButton(Buttons.LEFT,   null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_LEFT_BORDER = true, SHOW_TOP_BORDER = true, SHOW_BOTTOM_BORDER = true)
      DirectionButton(null,           null, GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.RIGHT,  null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_BOTTOM_BORDER = true)
    }
    Row(horizontalArrangement = Arrangement.Center)
    {
      DirectionButton(Buttons.DOWN, Buttons.LEFT,   GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.DOWN, null,           GAME_BOY, lastDirection, activeDpadButtons, SHOW_BOTTOM_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
      DirectionButton(Buttons.DOWN, Buttons.RIGHT,  GAME_BOY, lastDirection, activeDpadButtons)
    }
  }
}

/**
 * Renders the hardware input layout for the standard action keys (A and B).
 *
 * Uses interleaved invisible components to track sliding tap gestures or fat-finger
 * presses across buttons for seamless game responses.
 *
 * @param GAME_BOY The active [GameBoy] hardware wrapper engine receiving input updates.
 */
@Composable
fun GameBoyActionButtons(GAME_BOY: GameBoy)
{
  Column ()
  {
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
    {
      NormalButton("", Buttons.A, GAME_BOY, Buttons.B, true, IS_INVISIBLE = true)
      NormalButton(stringResource(R.string.btn_a), Buttons.A, GAME_BOY, null, true)
    }
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
    {
      NormalButton(stringResource(R.string.btn_b), Buttons.B, GAME_BOY, null, true)
      NormalButton("", Buttons.B, GAME_BOY, Buttons.A, true, IS_INVISIBLE = true)
    }
  }
}

/**
 * Complete tactile input dashboard for the device emulator control scheme.
 *
 * Includes the foundational directional cross-pad, secondary action switches, settings gateway,
 * and hardware navigation keys (Select / Start). Taps across the region keep interaction timers alive.
 *
 * @param GAME_BOY The active [GameBoy] hardware wrapper engine receiving physical register adjustments.
 * @param VIEW_MODEL The connected state coordinator controlling session workflows.
 * @param ON_INTERACTION Event dispatch triggered on user touch interactions to preserve background settings.
 * @param ON_SETTINGS_CLICK Event dispatch executing context shifts toward overlay options.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GameBoyControls(
  GAME_BOY   : GameBoy,
  VIEW_MODEL : EmulatorViewModel,
  ON_INTERACTION: () -> Unit,
  ON_SETTINGS_CLICK : () -> Unit)
{
  Column(
    horizontalAlignment = Alignment.CenterHorizontally,
    modifier            = Modifier
      .fillMaxWidth()
      .background(GameBoyColors.DarkGreen)
      .padding(top = 16.dp, bottom = 16.dp)
      .pointerInput(Unit)
      {
        detectTapGestures { ON_INTERACTION() }
      }
        )
  {
    Row (
      modifier              = Modifier.fillMaxWidth().padding(horizontal = 8.dp),
      horizontalArrangement = Arrangement.SpaceBetween,
      verticalAlignment     = Alignment.CenterVertically)
    {
      Box(modifier = Modifier.pointerInput(Unit)
      {
        awaitPointerEventScope()
        {
          while (true)
          {
            awaitPointerEvent()
            ON_INTERACTION()
          }
        }
      })
      { GameBoyDpad(GAME_BOY) }
      Box(modifier = Modifier.pointerInput(Unit)
      {
        awaitPointerEventScope()
        {
          while (true)
          {
            awaitPointerEvent()
            ON_INTERACTION()
          }
        }
      })
      { GameBoyActionButtons(GAME_BOY) }
    }

    Spacer(modifier = Modifier.height(24.dp))

    Icon(
      painter             = painterResource(R.drawable.settings),
      contentDescription  = stringResource(R.string.ingame_settings),
      tint                = GameBoyColors.MediumGreen,
      modifier            = Modifier
        .size(32.dp)
        .clickable
        {
          ON_INTERACTION()
          ON_SETTINGS_CLICK()
        })

    Spacer(modifier = Modifier.height(16.dp))

    Row(
      modifier                = Modifier.fillMaxWidth(),
      horizontalArrangement   = Arrangement.Center,
      verticalAlignment       = Alignment.CenterVertically)
    {
      NormalButton(stringResource(R.string.select_btn), Buttons.SELECT, GAME_BOY)
      Spacer(Modifier.width(32.dp))
      NormalButton(stringResource(R.string.start_btn), Buttons.START, GAME_BOY)
    }
  }
}

/**
 * Standard utility controller for rendering flat, styled retro buttons.
 *
 * Bypasses high-level onClick components in favor of custom down-press low-latency pointer bindings,
 * simulating hardware switch toggles by firing events to the hardware layer dynamically.
 *
 * @param LABEL Text displayed within the center of the control surface.
 * @param BUTTON The primary [Buttons] hardware mapping triggered upon input engagement.
 * @param GAME_BO_Y The active [GameBoy] hardware core parsing state signals.
 * @param SECONDARY Optional auxiliary key code tied to parallel press conditions. Defaults to null.
 * @param IS_SQUARE Modifies component aspect ratio between classic flat capsules and perfect square variants.
 * @param MODIFIER [Modifier] used to adjust layout parameters.
 * @param IS_INVISIBLE Configures the element to hide its visibility bounding box while retaining live hardware hit-scans.
 */
@Composable
fun NormalButton(
  LABEL         : String,
  BUTTON        : Buttons,
  GAME_BO_Y     : GameBoy,
  SECONDARY     : Buttons?  = null,
  IS_SQUARE     : Boolean   = false,
  MODIFIER      : Modifier  = Modifier,
  IS_INVISIBLE  : Boolean   = false)
{
  val isPressed : MutableState<Boolean> = remember { mutableStateOf(false) }
  val offset by animateDpAsState(
    if (isPressed.value && !IS_INVISIBLE) 2.dp
    else                                  0.dp,
    label = "offset")

  Button(
    onClick  = { /* ignored: handled via pointerInput */ },
    modifier = MODIFIER
      .size(
        if (IS_SQUARE)  64.dp
        else            96.dp,

        if (IS_SQUARE)  64.dp
        else            48.dp)
      .alpha(
        if (IS_INVISIBLE && !isPressed.value) 0f
        else                                  1f)
      .offset(x = offset, y = offset)
      .pointerInput(Unit)
      {
        awaitPointerEventScope()
        {
          while (true)
          {
            val event   = awaitPointerEvent()
            val pressed = event.changes.any { it.pressed }

            if (pressed && !isPressed.value)
            {
              isPressed.value = true
              GAME_BO_Y.sendButton(BUTTON, true)
              if (SECONDARY != null) GAME_BO_Y.sendButton(SECONDARY, true)
            }

            if (!pressed && isPressed.value)
            {
              isPressed.value = false
              GAME_BO_Y.sendButton(BUTTON, false)
              if (SECONDARY != null) GAME_BO_Y.sendButton(SECONDARY, false)
            }
          }
        }
      },
    shape  = RectangleShape,
    colors = ButtonColors(
      contentColor            = GameBoyColors.DarkGreen,
      containerColor          = GameBoyColors.MediumGreen.copy(
        alpha =
          if (SECONDARY != null) 0.0f
          else
          {
            if (isPressed.value) 0.6f
            else                 1.0f
          }),
      disabledContentColor    = GameBoyColors.DarkGreen,
      disabledContainerColor  = GameBoyColors.DarkGreen
                         ),
    border = BorderStroke(
      if (SECONDARY == null) 4.dp
      else                   0.dp,
      GameBoyColors.Green.copy(
        alpha =
          if (SECONDARY != null) 0.0f
          else                   1.0f
                              )),
    contentPadding = PaddingValues(0.dp))
  {
    if (SECONDARY == null)
    {
      Text(
        text        = LABEL,
        color       = GameBoyColors.LightGreen,
        fontSize    = 16.sp,
        fontFamily  = MinecraftFontFamily,
          )
    }
  }
}

/**
 * Dedicated segment button used to compose and calculate the directional control surface.
 *
 * Provides specialized boundary outline parameters to assemble the solid cross visual style
 * and handles localized touch capture scopes across standalone or composite angles.
 *
 * @param PRIMARY_BUTTON Main hardware direction index bound to this quadrant.
 * @param SECONDARY_BUTTON Auxiliary hardware direction index used for diagonal corner calculations.
 * @param GAME_BOY The target emulator core registry instance.
 * @param LAST_DIRECTION Mutable tracking container containing the most recent solo line event.
 * @param ACTIVE_DPAD_BUTTONS Set tracking reference identifying what button indices are held down concurrently.
 * @param SHOW_LEFT_BORDER Enables drawing a solid bounding line along the left canvas facade.
 * @param SHOW_TOP_BORDER Enables drawing a solid bounding line along the top canvas facade.
 * @param SHOW_RIGHT_BORDER Enables drawing a solid bounding line along the right canvas facade.
 * @param SHOW_BOTTOM_BORDER Enables drawing a solid bounding line along the bottom canvas facade.
 */
@Composable
fun DirectionButton(
  PRIMARY_BUTTON     : Buttons?,
  SECONDARY_BUTTON   : Buttons?,
  GAME_BOY           : GameBoy,
  LAST_DIRECTION     : MutableState<Buttons?>,     // - - - Shared state for last single direction
  ACTIVE_DPAD_BUTTONS: MutableState<Set<Buttons>>, // - - - Shared state for currently active buttons
  SHOW_LEFT_BORDER   : Boolean = false,
  SHOW_TOP_BORDER    : Boolean = false,
  SHOW_RIGHT_BORDER  : Boolean = false,
  SHOW_BOTTOM_BORDER : Boolean = false
                   )
{
  // - - - Determine if the button is invisible based on its type

  val isTopLeftCorner     = (PRIMARY_BUTTON == Buttons.UP   && SECONDARY_BUTTON == Buttons.LEFT)
  val isTopRightCorner    = (PRIMARY_BUTTON == Buttons.UP   && SECONDARY_BUTTON == Buttons.RIGHT)
  val isBottomLeftCorner  = (PRIMARY_BUTTON == Buttons.DOWN && SECONDARY_BUTTON == Buttons.LEFT)
  val isBottomRightCorner = (PRIMARY_BUTTON == Buttons.DOWN && SECONDARY_BUTTON == Buttons.RIGHT)
  val IS_INVISIBLE_CALCULATED = isTopLeftCorner || isTopRightCorner || isBottomLeftCorner || isBottomRightCorner
  var isPressed = remember { mutableStateOf(false) }
  val offset by animateDpAsState(if (isPressed.value && !IS_INVISIBLE_CALCULATED) 2.dp else 0.dp, label = "offset")

  Box(
    modifier = Modifier
      .size(64.dp)
      .alpha(
        if (IS_INVISIBLE_CALCULATED && !isPressed.value) 0f
        else                         1f)
      .offset(x = offset, y = offset)
      .pointerInput(PRIMARY_BUTTON, SECONDARY_BUTTON)
      {
        awaitPointerEventScope()
        {
          isPressed.value = false
          while (true)
          {
            val event             = awaitPointerEvent()
            val currentlyPressed  = event.changes.any { it.pressed }

            if (currentlyPressed && !isPressed.value)
            {
              isPressed.value     = true
              val buttonsToPress  = mutableSetOf<Buttons>()

              if (PRIMARY_BUTTON != null)
              {
                buttonsToPress.add(PRIMARY_BUTTON)
                if (SECONDARY_BUTTON == null) LAST_DIRECTION.value = PRIMARY_BUTTON
              }

              if (SECONDARY_BUTTON != null) buttonsToPress.add(SECONDARY_BUTTON);

              if (PRIMARY_BUTTON == null && SECONDARY_BUTTON == null)
              { LAST_DIRECTION.value?.let { buttonsToPress.add(it) } }

              // - - - Send presses
              buttonsToPress.forEach { GAME_BOY.sendButton(it, true) }
              ACTIVE_DPAD_BUTTONS.value = buttonsToPress
            }

            if (!currentlyPressed && isPressed.value)
            {
              isPressed.value = false
              ACTIVE_DPAD_BUTTONS.value.forEach { GAME_BOY.sendButton(it, false) }
              ACTIVE_DPAD_BUTTONS.value = emptySet()
            }
          }
        }
      }
      .background(GameBoyColors.MediumGreen.copy(alpha = if (isPressed.value) 0.6f else 1.0f))
     )
  {
    // - - - Only draw borders if the button is not invisible
    if (!IS_INVISIBLE_CALCULATED)
    {
      Canvas(modifier = Modifier.matchParentSize())
      {
        if (SHOW_LEFT_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(0f, 0f),
            end         = Offset(0f, size.height + 1),
            strokeWidth = 4.dp.toPx())
        if (SHOW_TOP_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(0f, 0f),
            end         = Offset(size.width + 1, 0f),
            strokeWidth = 4.dp.toPx())
        if (SHOW_RIGHT_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(size.width, 0f),
            end         = Offset(size.width + 1, size.height + 1),
            strokeWidth = 4.dp.toPx())
        if (SHOW_BOTTOM_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(0f, size.height),
            end         = Offset(size.width + 1, size.height + 1),
            strokeWidth = 4.dp.toPx())
      }
    }
  }
}