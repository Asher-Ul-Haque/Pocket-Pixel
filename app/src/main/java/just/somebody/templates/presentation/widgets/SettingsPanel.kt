package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
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
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.shadow
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
  ON_CLOSE    : () -> Unit
)
{
  var settingsPage by remember { mutableIntStateOf(0) }
  val settings     by EMULATOR.settings.collectAsState()

  GAME_BOY.pauseEmulator()

  ModalBottomSheet(
    modifier          = MODIFIER,
    onDismissRequest  =
      {
        ON_CLOSE()
        GAME_BOY.resumeEmulator()
      },
    containerColor = GameBoyColors.DarkGreen
  )
  {
    Column(
      modifier            = Modifier.padding(16.dp),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.CenterHorizontally
    )
    {
      CustomText("Game Settings", FONT_SIZE = 36)
      Spacer(modifier = Modifier.padding(8.dp))

      SettingsTabs(
        SELECTED = settingsPage,
        ON_SELECT = { settingsPage = it }
      )

      Spacer(modifier = Modifier.padding(8.dp))

      when (settingsPage)
      {
        0 -> LinkCableScreen(LINK_CABLE, Modifier.fillMaxSize(), false)

        1 -> AudioSettingsSection(settings.channelVolume)
        { vol, ch -> EMULATOR.setVolume(vol, ch) }

        2 -> VisualSettingsSection(
          SELECTED_INDEX = settings.paletteIndex,
          ON_COLOR_SELECTED = { EMULATOR.setPaletteIndex(it) }
        )

        3 -> SaveSettingsSection(
          ON_FLUSH  = { GAME_BOY.flushSave() },
          ON_DELETE = { GAME_BOY.deleteRamFile() }
        )
      }
    }
  }
}

@Composable
private fun SettingsTabs(SELECTED : Int, ON_SELECT : (Int) -> Unit)
{
  val tabs = listOf("Cable", "Audio", "Visual", "ROM")

  Column(
    modifier = Modifier
      .fillMaxWidth()
      .padding(bottom = 4.dp),
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    tabs.chunked(2).forEach()
    { rowItems ->
      Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
      )
      {
        rowItems.forEachIndexed()
        { i, title ->
          val index = tabs.indexOf(title)
          CustomButton(
            ON_CLICK = { ON_SELECT(index) },
            MODIFIER = Modifier
              .weight(1f)
          ) { CustomText(title) }
        }
        // - - - Fill remaining space if row has only one item
        if (rowItems.size == 1) Spacer(modifier = Modifier.weight(1f))
      }
      Spacer(modifier = Modifier.height(8.dp))
    }
  }
}


@Composable
private fun AudioSettingsSection(
  VOLUMES           : List<Float>,
  ON_VOLUME_CHANGE  : (Float, Int) -> Unit
) {
  val colors = SliderDefaults.colors(
    thumbColor          = GameBoyColors.Green,
    activeTrackColor    = GameBoyColors.Green,
    inactiveTrackColor  = GameBoyColors.MediumGreen
  )

  val labels = listOf(
    "Master Volume",
    "Pulse Channel 1 Volume",
    "Pulse Channel 2 Volume",
    "Wave Channel Volume",
    "Noise Channel Volume"
  )

  Column(horizontalAlignment = Alignment.CenterHorizontally)
  {
    labels.forEachIndexed()
    { index, label ->
      CustomText(label)
      Slider(
        value         = VOLUMES[index],
        onValueChange = { ON_VOLUME_CHANGE(it, index) },
        colors        = colors
      )
    }
  }
}

@Composable
fun VisualSettingsSection(
  SELECTED_INDEX : Int,
  ON_COLOR_SELECTED: (Int) -> Unit
)
{
  val options = listOf("Default", "DMG", "Pocket", "Purple", "Sepia", "Blue", "Teal", "Peach")
  var expanded by remember { mutableStateOf(false) }

  Box(
    modifier = Modifier
      .fillMaxWidth()
      .padding(vertical = 8.dp)
      .shadow(2.dp)
      .background(GameBoyColors.MediumGreen)
      .clickable { expanded = true }
      .padding(horizontal = 16.dp, vertical = 12.dp)
  ) {
    Row(
      verticalAlignment     = Alignment.CenterVertically,
      horizontalArrangement = Arrangement.SpaceBetween,
      modifier              = Modifier.fillMaxWidth()
    )
    {
      CustomText(options[SELECTED_INDEX])

      androidx.compose.material3.Icon(
        imageVector         = Icons.Default.ArrowDropDown,
        contentDescription  = "Dropdown Arrow",
        tint                = GameBoyColors.Green,
        modifier            = Modifier.rotate(if (expanded) 180f else 0f)
      )
    }

    DropdownMenu(
      expanded          = expanded,
      onDismissRequest  = { expanded = false },
      modifier          = Modifier.background(GameBoyColors.MediumGreen)
    )
    {
      options.forEachIndexed()
      { index, label ->
        DropdownMenuItem(
          text    = { CustomText(label) },
          onClick = {
            ON_COLOR_SELECTED(index)
            expanded = false
          }
        )
      }
    }
  }
}



@Composable
private fun SaveSettingsSection(
  ON_FLUSH  : () -> Unit,
  ON_DELETE : () -> Unit
)
{
  CustomButton(ON_CLICK = ON_FLUSH)
  { CustomText("Flush Save File Now") }

  CustomButton(
    ON_CLICK  = ON_DELETE,
    COLOR     = GameBoyColors.Error
  )
  { CustomText("Delete Save File") }
}

