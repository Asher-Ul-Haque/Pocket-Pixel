package just.somebody.templates.presentation.screens

import android.graphics.Bitmap
import android.net.Uri
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.BlendMode
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.toIntSize
import just.somebody.templates.App
import just.somebody.templates.domain.models.Game
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.withContext

@Composable
fun EmulatorScreen(
  MODIFIER   : Modifier = Modifier,
  VIEW_MODEL : EmulatorViewModel,
  URI        : String)
{


  val frameCount by VIEW_MODEL.frameSignal.collectAsState()

  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  Column (
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {

    Text("$frameCount")
    Canvas(
      modifier = Modifier.border(width = 4.dp, color = Color.Black)
        .fillMaxWidth()
        .aspectRatio(160f / 144f))
    {
      drawImage(
        image = App.appModule.gameBoy.getFrameBuffer().asImageBitmap(),
        dstSize = size.toIntSize()
      )

      val scanlineHeight = 4f // How thick each scanline is
      for (i in 0 until (size.height / scanlineHeight).toInt()) {
        val alpha = (0.3f + Math.sin((i.toFloat() * 0.25f).toDouble()) * 0.2f) // Vary the opacity slightly for effect
        drawLine(
          color = Color.Black.copy(alpha = alpha.toFloat()), // Black color for scanlines
          start = Offset(0f, i * scanlineHeight),
          end = Offset(size.width, i * scanlineHeight),
          strokeWidth = scanlineHeight
        )
      }

      // Optional: Add vignette effect (darken the edges)
      drawRect(
        color = Color.Black.copy(alpha = 0.4f),
        topLeft = Offset(0f, 0f),
        size = size,
        blendMode = BlendMode.DstIn
      )
    }
    GameBoyControls(App.appModule.gameBoy)
  }
}