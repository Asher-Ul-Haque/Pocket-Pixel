package just.somebody.templates.presentation.screens

import android.net.Uri
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.SnackbarHost
import androidx.compose.runtime.Composable
import androidx.compose.runtime.State
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.launch

@Composable
fun SettingsScreen(
  MODIFIFER  : Modifier  = Modifier,
  VIEW_MODEL : SettingsViewModel
)
{
  val settings = VIEW_MODEL.settings.collectAsState()
  val scope = rememberCoroutineScope()

  Column (
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally,
    modifier            = MODIFIFER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen)
      .padding(16.dp)
      .border(
        width = 4.dp,
        color = GameBoyColors.LightGreen
      )
      .background(GameBoyColors.MediumGreen)
  )
  {
    CustomButton(
      ON_CLICK = { TODO() },
      MODIFIER = Modifier
        .padding(16.dp)
        .fillMaxWidth()
    )
    {
      Column ()
      {
        CustomText(
          TEXT     = "Change ROM directory",
          MODIFIER = Modifier.padding(0.dp)
        )
        settings.value.externalUris["GAME_BOY_ROMS"]?.let()
        { rawUri ->
          val dirName =
            try   { Uri.parse(rawUri).lastPathSegment?.substringAfter("primary:") ?: "Unknown" }
            catch (e: Exception)
            {
              scope.launch()
              { SnackbarController.sendEvent(SnackbarEvent("failed to parse ROMs directory uri")) }
            }

          CustomText(
            TEXT      = dirName.toString(),
            MODIFIER  = Modifier.padding(0.dp),
            FONT_SIZE = 16
          )
        }
      }
    }

    CustomButton(
      ON_CLICK = { VIEW_MODEL.rescan() },
      MODIFIER = Modifier
        .padding(16.dp)
        .fillMaxWidth(),
    )
    { CustomText("Rescan ROMs") }

    CustomButton(
      ON_CLICK =
        {
          VIEW_MODEL.factoryReset()
        },
      MODIFIER = Modifier
        .padding(16.dp)
        .fillMaxWidth(),
      COLOR = GameBoyColors.Error
    )
    {
      CustomText("Factory Reset")
    }
  }
}