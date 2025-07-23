package just.somebody.templates.presentation.widgets

import android.view.MotionEvent
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
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
import just.somebody.templates.domain.Buttons
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.PokeFontFamily


@Composable
fun GameBoyControls(GAME_BOY : GameBoy)
{
  val lastDirection = remember { mutableStateOf<Buttons?>(null) }

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
          DirectionButton(Buttons.UP, GAME_BOY)
          DirectionButton(Buttons.UP, GAME_BOY, false, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
          DirectionButton(Buttons.UP, GAME_BOY)
        }
        Row(horizontalArrangement = Arrangement.Center)
        {
          DirectionButton(Buttons.LEFT,  GAME_BOY, false, SHOW_LEFT_BORDER = true, SHOW_TOP_BORDER = true, SHOW_BOTTOM_BORDER = true)
          DirectionButton(Buttons.LEFT,  GAME_BOY, false)
          DirectionButton(Buttons.RIGHT, GAME_BOY, false, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_BOTTOM_BORDER = true)
        }
        Row(horizontalArrangement = Arrangement.Center)
        {
          DirectionButton(Buttons.DOWN, GAME_BOY)
          DirectionButton(Buttons.DOWN, GAME_BOY, false, SHOW_BOTTOM_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
          DirectionButton(Buttons.DOWN, GAME_BOY)
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
      NormalButton("Start", Buttons.START, GAME_BOY)
      Spacer(Modifier.padding(16.dp))
      NormalButton("Select", Buttons.SELECT, GAME_BOY)
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
    onClick     =  { GAME_BOY.sendButton(BUTTON, true) },
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
  BUTTON             : Buttons,
  GAME_BOY           : GameBoy,
  IS_INVISIBLE       : Boolean = true,
  SHOW_LEFT_BORDER   : Boolean = false,
  SHOW_TOP_BORDER    : Boolean = false,
  SHOW_RIGHT_BORDER  : Boolean = false,
  SHOW_BOTTOM_BORDER : Boolean = false
) {
  Box(
    modifier = Modifier
      .size(64.dp)
      .alpha(
        if (IS_INVISIBLE) 0f
        else              1f)
      .pointerInteropFilter ()
      { motionEvent ->
        when (motionEvent.action)
        {
          MotionEvent.ACTION_DOWN -> GAME_BOY.sendButton(BUTTON, true)
          MotionEvent.ACTION_UP,
          MotionEvent.ACTION_CANCEL -> GAME_BOY.sendButton(BUTTON, false)
        }
        true
      }
      .background(GameBoyColors.MediumGreen)
  )
  {
    if (!IS_INVISIBLE)
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