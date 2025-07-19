package just.somebody.templates.presentation.screens

import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import just.somebody.templates.App
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun EmulatorScreen(
  MODIFIER: Modifier = Modifier,
  VIEW_MODEL: EmulatorViewModel,
  URI: String
) {
  LaunchedEffect(URI) {
    VIEW_MODEL.runEmulator(URI)
  }

  DisposableEffect(Unit) {
    onDispose {
      VIEW_MODEL.stopEmulator()
    }
  }

  Column(
    modifier = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen)
      .padding(top = 48.dp),
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  ) {
    // Use SurfaceView and set fixed size
    AndroidView(
      factory = { context ->
        SurfaceView(context).apply {
          holder.setFixedSize(160, 144) // Native Game Boy resolution

          holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
              VIEW_MODEL.prepareSurface(holder.surface)
            }

            override fun surfaceChanged(
              holder: SurfaceHolder,
              format: Int,
              width: Int,
              height: Int
            ) {
              // no-op
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
              VIEW_MODEL.prepareSurface(null)
            }
          })
        }
      },
      modifier = Modifier
        .fillMaxWidth()
        .aspectRatio(160f / 144f)
    )

    Spacer(modifier = Modifier.height(24.dp))

    GameBoyControls(App.appModule.gameBoy)
  }
}
