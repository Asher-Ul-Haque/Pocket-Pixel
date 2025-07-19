package just.somebody.templates.presentation.screens

import android.graphics.PixelFormat
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import just.somebody.templates.App
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun EmulatorScreen(
  MODIFIER    : Modifier = Modifier,
  VIEW_MODEL  : EmulatorViewModel,
  URI         : String
)
{
  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  Column(
    modifier = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen)
      .padding(top = 48.dp), // Top padding to shift content down
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  ) {
    Box(
      modifier = Modifier
        .padding(16.dp) // Space around emulator screen
        .shadow(8.dp, RoundedCornerShape(8.dp)) // Optional: soft shadow for depth
        .background(GameBoyColors.Green, RoundedCornerShape(8.dp))
        .border(4.dp, Color.Black, RoundedCornerShape(8.dp)) // Thick bezel
        .padding(12.dp) // Inner padding around the screen
    ) {
      AndroidView(
        factory = { context ->
          object : SurfaceView(context) {
            init {
              holder.setFormat(PixelFormat.RGB_565)
              holder.setFixedSize(160, 144)
              setWillNotDraw(false)

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
            setLayerType(View.LAYER_TYPE_SOFTWARE, null)
            background = null
            isFocusable = false
          }
        },
        modifier = Modifier
          .aspectRatio(160f / 144f)
          .fillMaxWidth()
      )
    }

    Spacer(modifier = Modifier.height(24.dp)) // Space between screen and controls

    GameBoyControls(App.appModule.gameBoy)
  }

}
