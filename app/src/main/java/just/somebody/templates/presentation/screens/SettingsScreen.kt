package just.somebody.templates.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import just.somebody.templates.R
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun SettingsScreen(
  VIEW_MODEL : SettingsViewModel,
  MODIFIFER  : Modifier = Modifier
)
{
  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen),
    contentAlignment = Alignment.Center)
  {
    CustomText(
      TEXT      = stringResource(R.string.SETTINGS),
      FONT_SIZE = 21)
  }
}
