package just.somebody.templates.presentation.widgets

import android.view.MotionEvent
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.absoluteOffset
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.input.pointer.pointerInteropFilter
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.App
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.PokeFontFamily


@Composable
fun GameBoyControls(GAME_BOY : GameBoy)
{
  // - - - Stores the last successfully pressed single directional button (UP, DOWN, LEFT, RIGHT)
  val lastDirection = remember { mutableStateOf<Buttons?>(null) }
  // - - - Tracks all directional buttons currently active due to fat-finger logic (e.g., UP and LEFT)
  val activeDpadButtons = remember { mutableStateOf(setOf<Buttons>()) }

  Column(
    horizontalAlignment = Alignment.CenterHorizontally,
    verticalArrangement = Arrangement.SpaceBetween,
    modifier            = Modifier.fillMaxSize().padding(top = 16.dp, bottom = 16.dp)
  )
  {
    Row (horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically)
    {
      Column(
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
      )
      {
        Row(horizontalArrangement = Arrangement.Center)
        {
          // - - - Top-Left Corner
          DirectionButton(Buttons.UP, Buttons.LEFT, GAME_BOY, lastDirection, activeDpadButtons)
          // - - - Top Edge
          DirectionButton(Buttons.UP, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
          // - - - Top-Right Corner
          DirectionButton(Buttons.UP, Buttons.RIGHT, GAME_BOY, lastDirection, activeDpadButtons)
        }
        Row(horizontalArrangement = Arrangement.Center)
        {
          // - - - Left Edge
          DirectionButton(Buttons.LEFT,  null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_LEFT_BORDER = true, SHOW_TOP_BORDER = true, SHOW_BOTTOM_BORDER = true)
          // - - - Center
          DirectionButton(null, null, GAME_BOY, lastDirection, activeDpadButtons)
          // - - - Right Edge
          DirectionButton(Buttons.RIGHT, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_BOTTOM_BORDER = true)
        }
        Row(horizontalArrangement = Arrangement.Center)
        {
          // - - - Bottom-Left Corner
          DirectionButton(Buttons.DOWN, Buttons.LEFT, GAME_BOY, lastDirection, activeDpadButtons)
          // - - - Bottom Edge
          DirectionButton(Buttons.DOWN, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_BOTTOM_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
          // - - - Bottom-Right Corner
          DirectionButton(Buttons.DOWN, Buttons.RIGHT, GAME_BOY, lastDirection, activeDpadButtons)
        }
      }
      Spacer(Modifier.padding(16.dp))
      Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
      {
        NormalButton("B", Buttons.B, GAME_BOY, true, Modifier.absoluteOffset(y = (64).dp))
        NormalButton("A", Buttons.A, GAME_BOY, true)
      }
    }

    Row(
      modifier                = Modifier.fillMaxWidth(),
      horizontalArrangement   = Arrangement.Center,
      verticalAlignment       = Alignment.CenterVertically)
    {
      NormalButton("Select", Buttons.SELECT, GAME_BOY)
      Spacer(Modifier.padding(16.dp))
      NormalButton("Start", Buttons.START, GAME_BOY)
    }
  }
}


@Composable
fun NormalButton(
  LABEL     : String,
  BUTTON    : Buttons,
  GAME_BOY  : GameBoy,
  IS_SQUARE : Boolean   = false,
  MODIFIER  : Modifier  = Modifier)
{
  Button(
    onClick     =  { /* Handled by pointerInteropFilter */ },
    modifier    = MODIFIER
      .width (
        if (IS_SQUARE) 64.dp
        else           96.dp)
      .height(
        if (IS_SQUARE) 64.dp
        else           48.dp)
      .pointerInteropFilter()
      { motionEvent ->
        when (motionEvent.action)
        {
          MotionEvent.ACTION_DOWN   -> GAME_BOY.sendButton(BUTTON, true)
          MotionEvent.ACTION_UP,
          MotionEvent.ACTION_CANCEL -> GAME_BOY.sendButton(BUTTON, false)
        }
        true
      },
    shape       = RectangleShape,
    colors      = ButtonColors(
      contentColor            = GameBoyColors.DarkGreen,
      containerColor          = GameBoyColors.MediumGreen,
      disabledContentColor    = Color.Gray,
      disabledContainerColor  = Color.Gray
    ),
    border      = BorderStroke(4.dp, GameBoyColors.Green),
  )
  {
    Text(
      text        = LABEL,
      color       = GameBoyColors.LightGreen,
      fontSize    = 16.sp,
      fontFamily  = PokeFontFamily,
    )
  }
}

@Composable
fun DirectionButton(
  PRIMARY_BUTTON     : Buttons?,
  SECONDARY_BUTTON   : Buttons?,
  GAME_BOY           : GameBoy,
  LAST_DIRECTION     : MutableState<Buttons?>,     // - - - Shared state for last single direction
  ACTIVE_DPAD_BUTTONS: MutableState<Set<Buttons>>, // - - - Shared state for currently active buttons
  IS_INVISIBLE       : Boolean = true,
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
  val isCenter            = (PRIMARY_BUTTON == null         && SECONDARY_BUTTON == null)

  val IS_INVISIBLE_CALCULATED = isTopLeftCorner || isTopRightCorner || isBottomLeftCorner || isBottomRightCorner

  Box(
    modifier = Modifier
      .size(64.dp)
      .alpha(
        if (IS_INVISIBLE_CALCULATED) 0f
        else                         1f)
      .pointerInteropFilter ()
      { motionEvent ->
        when (motionEvent.action)
        {
          MotionEvent.ACTION_DOWN ->
          {
            // - - - Release any previously active button
            ACTIVE_DPAD_BUTTONS.value.forEach { button -> GAME_BOY.sendButton(button, false) }
            ACTIVE_DPAD_BUTTONS.value = emptySet()

            // - - - Determine which buttons to press based on PRIMARY_BUTTON and SECONDARY_BUTTON
            val buttonsToPress = mutableSetOf<Buttons>()

            if (PRIMARY_BUTTON != null)
            {
              buttonsToPress.add(PRIMARY_BUTTON)
              if (SECONDARY_BUTTON == null) { LAST_DIRECTION.value = PRIMARY_BUTTON }
            }

            if (SECONDARY_BUTTON != null) { buttonsToPress.add(SECONDARY_BUTTON) }

            // - - - Special case for the center button
            if (PRIMARY_BUTTON == null && SECONDARY_BUTTON == null)
            {
              LAST_DIRECTION.value?.let { lastDir -> buttonsToPress.add(lastDir) }
            }

            // - - - Press the determined buttons and update activeDpadButtons
            buttonsToPress.forEach { button -> GAME_BOY.sendButton(button, true) }
            ACTIVE_DPAD_BUTTONS.value = buttonsToPress.toSet()
            true
          }
          MotionEvent.ACTION_UP,
          MotionEvent.ACTION_CANCEL ->
          {
            // - - - Release all currently active buttons - - -
            ACTIVE_DPAD_BUTTONS.value.forEach { button ->
              GAME_BOY.sendButton(button, false)
            }
            ACTIVE_DPAD_BUTTONS.value = emptySet() // - - - Clear the set - - -
            true
          }
          else -> false
        }
      }
      .background(GameBoyColors.MediumGreen)
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
            strokeWidth = 4.dp.toPx()
          )
        if (SHOW_TOP_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(0f, 0f),
            end         = Offset(size.width + 1, 0f),
            strokeWidth = 4.dp.toPx()
          )
        if (SHOW_RIGHT_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(size.width, 0f),
            end         = Offset(size.width + 1, size.height + 1),
            strokeWidth = 4.dp.toPx()
          )
        if (SHOW_BOTTOM_BORDER)
          drawLine(
            color       = GameBoyColors.Green,
            start       = Offset(0f, size.height),
            end         = Offset(size.width + 1, size.height + 1),
            strokeWidth = 4.dp.toPx()
          )
      }
    }
  }
}
