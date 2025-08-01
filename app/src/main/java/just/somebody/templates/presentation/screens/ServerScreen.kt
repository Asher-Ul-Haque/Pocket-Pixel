package just.somebody.templates.presentation.screens

import android.content.Intent
import android.net.Uri
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.Icon
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.widgets.CustomButton
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

    CustomButton(
      ON_CLICK =
        {
          val githubUrl = "https://github.com/Asher-Ul-Haque/Pocket-Pixel"
          val intent    = Intent(Intent.ACTION_VIEW, Uri.parse(githubUrl)).apply { addFlags(Intent.FLAG_ACTIVITY_NEW_TASK) }
          App.appModule.context.startActivity(intent)
        },
    )
    {
      Icon(
        painter            = painterResource(R.drawable.github),
        contentDescription = "Check information",
        tint                = Color.Black,
        modifier            = Modifier
          .size(64.dp)
          .padding(8.dp)
      )
    }
  }
}