package just.somebody.templates.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import just.somebody.templates.App
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.viewModels.LinkCableViewModel
import just.somebody.templates.presentation.widgets.GameBoyControls
import just.somebody.templates.presentation.widgets.GameBoyFrame

@Composable
fun EmulatorScreen(
  MODIFIER    : Modifier = Modifier,
  VIEW_MODEL  : EmulatorViewModel,
  LINK_CABLE  : LinkCableViewModel,
  URI         : String
)
{
  LaunchedEffect(URI) { VIEW_MODEL.runEmulator(URI) }

  DisposableEffect(Unit)
  {
    onDispose { VIEW_MODEL.stopEmulator() }
  }

  Column(
    modifier            = MODIFIER
      .fillMaxSize()
      .background(Color.Black)
      .padding(top = 48.dp),
    verticalArrangement = Arrangement.Bottom,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    val gameBoyAspectRatio = 160f / 144f

    AndroidView(
      modifier  = Modifier
        .fillMaxWidth()
        .aspectRatio(gameBoyAspectRatio),
      factory   =
        { context ->
          val gameBoySurfaceView = GameBoyFrame(context)
          gameBoySurfaceView
        },
      update    = { }
    )

    Spacer(modifier = Modifier.height(48.dp))

    GameBoyControls(App.appModule.gameBoy, VIEW_MODEL, LINK_CABLE)
  }
}