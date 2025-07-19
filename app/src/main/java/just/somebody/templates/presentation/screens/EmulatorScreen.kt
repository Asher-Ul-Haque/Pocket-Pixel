package just.somebody.templates.presentation.screens

import android.graphics.Bitmap
import android.graphics.PixelFormat
import android.net.Uri
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
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
import androidx.compose.runtime.DisposableEffect
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
import androidx.compose.ui.viewinterop.AndroidView
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
  MODIFIER: Modifier = Modifier,
  VIEW_MODEL: EmulatorViewModel,
  URI: String
) {
  LaunchedEffect(URI) {
    VIEW_MODEL.runEmulator(URI)
  }

  Column(
    modifier = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  ) {
    AndroidView(
      factory = { context ->
        object : SurfaceView(context) {
          init {
            holder.setFormat(PixelFormat.RGB_565) // ✅ Use RGB_565 format
            holder.setFixedSize(160, 144)         // ✅ Match Game Boy resolution
            setWillNotDraw(false)                 // ✅ Allow native drawing

            holder.addCallback(object : SurfaceHolder.Callback {
              override fun surfaceCreated(holder: SurfaceHolder) {
                VIEW_MODEL.setNativeSurface(holder.surface)
              }

              override fun surfaceDestroyed(holder: SurfaceHolder) {
                VIEW_MODEL.setNativeSurface(null)
              }

              override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}
            })
          }
        }.apply {
          setLayerType(View.LAYER_TYPE_SOFTWARE, null) // ✅ Disable smoothing
          background = null
          isFocusable = false
        }
      },
      modifier = Modifier
        .border(4.dp, Color.Black)
        .fillMaxWidth()
        .aspectRatio(160f / 144f) // ✅ Keeps proper 1:1 pixel scaling
    )

    GameBoyControls(App.appModule.gameBoy)
  }
}
