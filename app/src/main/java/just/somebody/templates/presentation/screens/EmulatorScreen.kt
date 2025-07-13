package just.somebody.templates.presentation.screens

import android.graphics.Bitmap
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.domain.models.Game
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.delay

@Composable
fun EmulatorScreen(
  MODIFIER : Modifier = Modifier,
  URI      : String)
{
  val gameBoy = App.appModule.gameBoy
  var bitmap by remember { mutableStateOf<Bitmap?>(null) }

  LaunchedEffect(Unit)
  {
    gameBoy.resetEmulator()
    gameBoy.loadROM(URI)
    gameBoy.startEmulator()

    while (true)
    {
      val frameStart = System.currentTimeMillis()
      bitmap         = gameBoy.getFrameBuffer()
      gameBoy.stepFrame()

      val target     = 16L // 60 GPS -> 16ms per frame
      val elapsed    = System.currentTimeMillis() - frameStart
      val sleepTime  = target - elapsed

      if (sleepTime > 0) delay(sleepTime)
    }
  }

  Column (
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    Box(
      modifier = Modifier
        .border(width = 8.dp, color = Color.Black)
        .fillMaxWidth()
        .aspectRatio(160f / 144f)
    )
    {
      bitmap?.let ()
      { bmp ->
        Image(
          bitmap              = bmp.asImageBitmap(),
          contentDescription  = null,
          modifier            = Modifier.fillMaxSize(),
          contentScale        = ContentScale.FillBounds
        )
      }
    }
    GameBoyControls(gameBoy)
  }
}
