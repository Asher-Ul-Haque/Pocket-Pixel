package just.somebody.templates.presentation.screens

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import just.somebody.templates.R
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun ServerScreen(MODIFIER : Modifier)
{
  Column (
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Top,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    CustomText(
      TEXT      = stringResource(R.string.LEGAL),
      FONT_SIZE = 44)

    Image(
      painter            = painterResource(R.drawable.no_internet),
      modifier           = Modifier.fillMaxSize(0.3f),
      contentDescription = null
    )

    CustomText(stringResource(R.string.PIRACY))
  }
}