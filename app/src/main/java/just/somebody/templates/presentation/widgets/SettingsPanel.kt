package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.unit.dp
import just.somebody.templates.domain.GameBoy
import just.somebody.templates.presentation.screens.LinkCableScreen
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.viewModels.LinkCableViewModel
import just.somebody.templates.ui.theme.GameBoyColors

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsPanel(
  MODIFIER    : Modifier = Modifier,
  GAME_BOY    : GameBoy,
  EMULATOR    : EmulatorViewModel,
  LINK_CABLE  : LinkCableViewModel,
  ON_CLOSE    : () -> Unit)
{
  var settingsPage by remember { mutableIntStateOf(0) }
  val settings          = EMULATOR.settings.collectAsState()
  GAME_BOY.pauseEmulator()
  ModalBottomSheet(
    modifier         = MODIFIER,
    onDismissRequest = { ON_CLOSE(); GAME_BOY.resumeEmulator(); },
    containerColor   = GameBoyColors.DarkGreen)
  {
    Column(
      modifier            = Modifier.padding(16.dp),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.CenterHorizontally
    )
    {
      CustomText(
        TEXT = "Game Settings",
        FONT_SIZE = 36)
      Spacer(modifier = Modifier.padding(8.dp))

      Row (
        modifier              = Modifier
          .fillMaxWidth()
          .shadow(
            elevation    = 4.dp,
            shape        = RectangleShape,
            ambientColor = Color.Black,
            spotColor    = Color.Black
          )
          .padding(bottom = 4.dp),
        horizontalArrangement = Arrangement.Start,
        verticalAlignment     = Alignment.CenterVertically
      )
      {
        CustomButton(ON_CLICK = { settingsPage = 0})
        { CustomText("Link Cable Settings") }

        CustomButton(
          ON_CLICK = { settingsPage = 1})
        { CustomText("Audio Settings") }

        CustomButton(
          ON_CLICK = { settingsPage = 2})
        { CustomText("Visual Settings") }

        CustomButton(
          ON_CLICK = { settingsPage = 3})
        { CustomText("Save Game Settings") }
      }

      Spacer(modifier = Modifier.padding(8.dp))

      when (settingsPage)
      {
        0 ->
          {
            Column (
              modifier = Modifier.fillMaxSize(),
              horizontalAlignment = Alignment.CenterHorizontally
            )
            {
              CustomButton(ON_CLICK = { GAME_BOY.flushSave() })
              { CustomText("Flush Save File Now") }
              LinkCableScreen(LINK_CABLE, Modifier.fillMaxSize())
            }
          }

        1 ->
          {
            val colors = SliderDefaults.colors(
              thumbColor          = GameBoyColors.Green,
              activeTrackColor    = GameBoyColors.Green,
              inactiveTrackColor  = GameBoyColors.MediumGreen)

            CustomText("Master Volume")
            Slider(
              value         = settings.value.channelVolume[0],
              onValueChange = { EMULATOR.setVolume(it, 0) },
              colors        = colors
            )

            CustomText("Pulse Channel 1 Volume")
            Slider(
              value         = settings.value.channelVolume[1],
              onValueChange = { EMULATOR.setVolume(it, 1) },
              colors        = colors
            )

            CustomText("Pulse Channel 2 Volume")
            Slider(
              value         = settings.value.channelVolume[2],
              onValueChange = { EMULATOR.setVolume(it, 2) },
              colors        = colors
            )

            CustomText("Wave Channel Volume")
            Slider(
              value         = settings.value.channelVolume[3],
              onValueChange = { EMULATOR.setVolume(it, 3) },
              colors        = colors
            )

            CustomText("Noise Channel Volume")
            Slider(
              value         = settings.value.channelVolume[4],
              onValueChange = { EMULATOR.setVolume(it, 4) },
              colors        = colors
            )
          }

        2 ->
         {

         }
      }
    }
  }
}