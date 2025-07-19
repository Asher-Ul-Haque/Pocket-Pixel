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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import just.somebody.templates.App
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.presentation.widgets.GameBoyFrame
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
  )
  {
    // - - - GameBoy GLSurfaceView here
    // Game Boy screen resolution: 160x144. Aspect ratio = 160f / 144f
    val gameBoyAspectRatio = 160f / 144f

    AndroidView(
      modifier = Modifier
        .fillMaxWidth(0.9f) // Take up 90% of the available width
        .aspectRatio(gameBoyAspectRatio) // Maintain the Game Boy's aspect ratio
        .background(Color.Black), // Placeholder background
      factory = { context ->
        // Create an instance of your custom GLSurfaceView
        val gameBoySurfaceView = GameBoyFrame(context)

        // Pass the GameBoy instance to the GLSurfaceView so it can interact with native methods

        gameBoySurfaceView
      },
      update = { view ->
        // This block is called when the state of the AndroidView changes.
        // You might use it for dynamic updates if needed, but for a GLSurfaceView,
        // most interaction is handled internally via its Renderer.
      }
    )

    Spacer(modifier = Modifier.height(24.dp))

    GameBoyControls(App.appModule.gameBoy)
  }
}