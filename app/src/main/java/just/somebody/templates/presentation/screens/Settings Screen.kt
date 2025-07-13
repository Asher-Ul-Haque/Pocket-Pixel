package just.somebody.templates.presentation.screens

import android.net.Uri
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.launch


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
  MODIFIFER   : Modifier = Modifier,
  VIEW_MODEL  : SettingsViewModel,
  ON_DISMISS  : () -> Unit,
)
{
  val settings      = VIEW_MODEL.settings.collectAsState()
  val scope         = rememberCoroutineScope()
  val pickDirectory = App.appModule.externalStorageManager.DirectoryPickerLauncher("GAME_BOY_ROMS")
  { uri ->
    if (uri != null) VIEW_MODEL.rescan()
    else             scope.launch()
    { SnackbarController.sendEvent(SnackbarEvent(message = "No directory picked")) }
  }

  ModalBottomSheet(
    onDismissRequest = ON_DISMISS,
    containerColor = GameBoyColors.DarkGreen)
  {
    Column(
      modifier            = Modifier.padding(16.dp),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.CenterHorizontally
    )
    {
      CustomText("Settings")
      CustomButton(
        ON_CLICK = pickDirectory,
        MODIFIER = Modifier
          .padding(0.dp)
          .fillMaxWidth()
      )
      {
        CustomText("Change ROM directory")
      }

      CustomButton(
        ON_CLICK = { VIEW_MODEL.rescan() },
        MODIFIER = Modifier
          .padding(0.dp)
          .fillMaxWidth(),
      )
      { CustomText("Rescan ROMs") }

      CustomButton(
        ON_CLICK = { VIEW_MODEL.factoryReset() },
        MODIFIER = Modifier
          .padding(0.dp)
          .fillMaxWidth(),
        COLOR = GameBoyColors.Error
      )
      { CustomText("Factory Reset") }
    }
  }
}