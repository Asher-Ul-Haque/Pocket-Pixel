package org.just_somebody.pocket_pixel

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.delay
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import org.just_somebody.pocket_pixel.emulatorScreen.domain.GameBoy
import org.just_somebody.pocket_pixel.emulatorScreen.presentation.EmulatorViewModel

@Composable
fun EmulatorScreen(
    MODIFIER    : Modifier          = Modifier,
    VIEW_MODEL  : EmulatorViewModel = EmulatorViewModel()
)
{
    val gameBoy     =   VIEW_MODEL.state.gameBoy
    var frame       by remember { mutableStateOf(renderFrame(gameBoy.getFrameBuffer())) }

    LaunchedEffect(Unit)
    {
        while (true)
        {
            frame = renderFrame(gameBoy.getFrameBuffer())
            delay(16L)
        }
    }


    Column(
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Top,
        horizontalAlignment = Alignment.CenterHorizontally,
    )
    {
        // - - - GameBoy Screen
        Canvas(
            modifier = Modifier
                .border(width = 8.dp, color = Color.Black)
                .fillMaxWidth()
                .aspectRatio(160f / 144f)
        ) { drawGameFrame(frame) }
        Spacer(modifier = Modifier.height(16.dp))
        GameBoyControls(gameBoy)
    }
}

private fun renderFrame(BUFFER: ByteArray): Array<Array<Color>>
{
    val width   = 160
    val height  = 144
    val colors  = Array(height) { Array(width) { Color.Black } }

    var pixelIndex = 0
    for (byte in BUFFER)
    {
        for (shift in 6 downTo 0 step 2)
        {
            val colorIndex  = (byte.toInt() shr shift) and 0b11
            val row         = pixelIndex / width
            val col         = pixelIndex % width
            colors[row][col] = mapColor(colorIndex)
            pixelIndex++
        }
    }
    return colors
}

private fun mapColor(COLOR_INDEX: Int): Color
{
    return when (COLOR_INDEX)
    {
        0       -> Color.Black
        1       -> Color.DarkGray
        2       -> Color.LightGray
        3       -> Color.White
        else    -> Color.Yellow
    }
}

private fun DrawScope.drawGameFrame(FRAME   : Array<Array<Color>>)
{
    val scaleX = size.width / 160f
    val scaleY = size.height / 144f

    FRAME.forEachIndexed()
    { rowIndex, row ->
        row.forEachIndexed()
        { colIndex, color ->
            drawRect(
                color   = color,
                topLeft = Offset(colIndex * scaleX, rowIndex * scaleY),
                size    = androidx.compose.ui.geometry.Size(scaleX, scaleY)
            )
        }
    }
}

@Composable
fun GameBoyControls(GAME_BOY : GameBoy)
{
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
                    DirectionButton(GameBoy.Buttons.BUTTON_UP, GAME_BOY)
                    DirectionButton(GameBoy.Buttons.BUTTON_UP, GAME_BOY, false, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
                    DirectionButton(GameBoy.Buttons.BUTTON_UP, GAME_BOY)
                }
                Row(horizontalArrangement = Arrangement.Center)
                {
                    DirectionButton(GameBoy.Buttons.BUTTON_LEFT,  GAME_BOY, false, SHOW_LEFT_BORDER = true, SHOW_TOP_BORDER = true, SHOW_BOTTOM_BORDER = true)
                    DirectionButton(GameBoy.Buttons.BUTTON_LEFT,  GAME_BOY, false)
                    DirectionButton(GameBoy.Buttons.BUTTON_RIGHT, GAME_BOY, false, SHOW_TOP_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_BOTTOM_BORDER = true)
                }
                Row(horizontalArrangement = Arrangement.Center)
                {
                    DirectionButton(GameBoy.Buttons.BUTTON_DOWN, GAME_BOY)
                    DirectionButton(GameBoy.Buttons.BUTTON_DOWN, GAME_BOY, false, SHOW_BOTTOM_BORDER = true, SHOW_RIGHT_BORDER = true, SHOW_LEFT_BORDER = true)
                    DirectionButton(GameBoy.Buttons.BUTTON_DOWN, GAME_BOY)
                }
            }
            Spacer(Modifier.padding(16.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
            {
                normalButton(
                    "B",
                    GameBoy.Buttons.BUTTON_B,
                    GAME_BOY,
                    true,
                    Modifier.absoluteOffset(y = (64).dp))
                normalButton(
                    "A",
                    GameBoy.Buttons.BUTTON_A,
                    GAME_BOY,
                    true)
            }
        }

        Row(
            modifier                = Modifier.fillMaxWidth(),
            horizontalArrangement   = Arrangement.Center,
            verticalAlignment       = Alignment.CenterVertically)
        {
            normalButton("Start", GameBoy.Buttons.BUTTON_START, GAME_BOY)
            Spacer(Modifier.padding(16.dp))
            normalButton("Select", GameBoy.Buttons.BUTTON_SELECT, GAME_BOY)
        }
    }

    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_SELECT, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_START, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_A, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_B, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_UP, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_DOWN, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_LEFT, false)
    GAME_BOY.sendButton(GameBoy.Buttons.BUTTON_RIGHT, false)
}

@Composable
fun normalButton(
    LABEL       : String,
    BUTTON      : GameBoy.Buttons,
    GAME_BOY    : GameBoy,
    IS_SQUARE   : Boolean   = false,
    MODIFIER    : Modifier  = Modifier)
{
    Button(
        onClick     =  { GAME_BOY.sendButton(BUTTON, true) },
        modifier    = MODIFIER
            .width(if (IS_SQUARE) 64.dp else 96.dp)
            .height(if (IS_SQUARE) 64.dp else 32.dp),
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
            fontFamily  = PokeFontFamily(),
        )
    }
}

@Composable
fun DirectionButton(
    BUTTON              : GameBoy.Buttons,
    GAME_BOY            : GameBoy,
    IS_INVISIBLE        : Boolean = true,
    SHOW_LEFT_BORDER    : Boolean = false,
    SHOW_TOP_BORDER     : Boolean = false,
    SHOW_RIGHT_BORDER   : Boolean = false,
    SHOW_BOTTOM_BORDER  : Boolean = false
)
{
    Box ()
    {
        Button(
            onClick     = { GAME_BOY.sendButton(BUTTON, true) },
            colors      = ButtonColors(
                containerColor          = GameBoyColors.MediumGreen,
                contentColor            = GameBoyColors.DarkGreen,
                disabledContainerColor  = Color.Gray,
                disabledContentColor    = Color.Gray
            ),
            modifier    = Modifier
                .size(64.dp)
                .alpha(if (IS_INVISIBLE) 0f else 1f),
            shape = RectangleShape
        ) {}

        if (!IS_INVISIBLE)
        {
            Canvas(modifier = Modifier.matchParentSize())
            {
                if (SHOW_LEFT_BORDER)
                {
                    drawLine(
                        color       = GameBoyColors.Green,
                        start       = Offset(0f, 0f),
                        end         = Offset(0f, size.height + 1),
                        strokeWidth = 4.dp.toPx()
                    )
                }
                if (SHOW_TOP_BORDER)
                {
                    drawLine(
                        color       = GameBoyColors.Green,
                        start       = Offset(0f, 0f),
                        end         = Offset(size.width + 1, 0f),
                        strokeWidth = 4.dp.toPx()
                    )
                }
                if (SHOW_RIGHT_BORDER)
                {
                    drawLine(
                        color       = GameBoyColors.Green,
                        start       = Offset(size.width, 0f),
                        end         = Offset(size.width + 1, size.height + 1),
                        strokeWidth = 4.dp.toPx()
                    )
                }
                if (SHOW_BOTTOM_BORDER)
                {
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
}