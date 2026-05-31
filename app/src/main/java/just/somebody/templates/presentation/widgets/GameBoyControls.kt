package just.somebody.templates.presentation.widgets

import androidx.compose.animation.core.animateDpAsState
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.ExperimentalMaterial3Api
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
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily


@Composable
fun GameBoyDpad(GAME_BOY: GameBoy) {
  val lastDirection     = remember { mutableStateOf<Buttons?>(null) }
  val activeDpadButtons = remember { mutableStateOf(setOf<Buttons>()) }

  Column(
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally,
  ) {
    Row(horizontalArrangement = Arrangement.Center) {
      DirectionButton(Buttons.UP, Buttons.LEFT, GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.UP, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
      DirectionButton(Buttons.UP, Buttons.RIGHT, GAME_BOY, lastDirection, activeDpadButtons)
    }
    Row(horizontalArrangement = Arrangement.Center) {
      DirectionButton(Buttons.LEFT,  null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_LEFT_BORDER = true, SHOW_TOP_BORDER = true, SHOW_BOTTOM_BORDER = true)
      DirectionButton(null, null, GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.RIGHT, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_BOTTOM_BORDER = true)
    }
    Row(horizontalArrangement = Arrangement.Center) {
      DirectionButton(Buttons.DOWN, Buttons.LEFT, GAME_BOY, lastDirection, activeDpadButtons)
      DirectionButton(Buttons.DOWN, null, GAME_BOY, lastDirection, activeDpadButtons, SHOW_BOTTOM_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
      DirectionButton(Buttons.DOWN, Buttons.RIGHT, GAME_BOY, lastDirection, activeDpadButtons)
    }
  }
}

@Composable
fun GameBoyActionButtons(GAME_BOY: GameBoy) {
  Column ()
  {
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
    {
      NormalButton("B", Buttons.B, GAME_BOY, null, true)
      NormalButton("", Buttons.A, GAME_BOY, Buttons.B, true, IS_INVISIBLE = true)
    }
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
    {
      NormalButton("", Buttons.B, GAME_BOY, Buttons.A, true, IS_INVISIBLE = true)
      NormalButton("A", Buttons.A, GAME_BOY, null, true)
    }
  }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GameBoyControls(
  GAME_BOY   : GameBoy,
  VIEW_MODEL : EmulatorViewModel)
{
  Column(
    horizontalAlignment = Alignment.CenterHorizontally,
    modifier            = Modifier
      .fillMaxWidth()
      .background(GameBoyColors.DarkGreen)
      .padding(top = 16.dp, bottom = 16.dp)
  )
  {
    Row (
      modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp),
      horizontalArrangement = Arrangement.SpaceEvenly, 
      verticalAlignment = Alignment.CenterVertically
    )
    {
      GameBoyDpad(GAME_BOY)
      GameBoyActionButtons(GAME_BOY)
    }

    Spacer(modifier = Modifier.padding(10.dp))

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
  SECONDARY : Buttons?  = null,
  IS_SQUARE : Boolean   = false,
  MODIFIER  : Modifier  = Modifier,
  IS_INVISIBLE : Boolean = false
)
{
  val isPressed = remember { mutableStateOf(false) }
  val offset by animateDpAsState(if (isPressed.value && !IS_INVISIBLE) 2.dp else 0.dp, label = "offset")

  Button(
    onClick  = { /* ignored: handled via pointerInput */ },
    modifier = MODIFIER
      .width(if (IS_SQUARE) 64.dp else 96.dp)
      .height(if (IS_SQUARE) 64.dp else 48.dp)
      .alpha(if (IS_INVISIBLE && !isPressed.value) 0f else 1f)
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
              GAME_BOY.sendButton(BUTTON, true)
              if (SECONDARY != null) GAME_BOY.sendButton(SECONDARY, true)
            }

            if (!pressed && isPressed.value)
            {
              isPressed.value = false
              GAME_BOY.sendButton(BUTTON, false)
              if (SECONDARY != null) GAME_BOY.sendButton(SECONDARY, false)
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
      disabledContentColor    = Color.Gray,
      disabledContainerColor  = Color.Gray
    ),
    border = BorderStroke(
      if (SECONDARY == null) 4.dp
      else                   0.dp,
      GameBoyColors.Green.copy(
        alpha =
          if (SECONDARY != null) 0.0f
          else                   1.0f
      )),
  )
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
