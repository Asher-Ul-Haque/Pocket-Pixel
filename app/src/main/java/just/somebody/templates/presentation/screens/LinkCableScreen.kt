package just.somebody.templates.presentation.screens

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.viewModels.LinkCableViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.TextInp
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
fun LinkCableScreen(
  VIEW_MODEL : LinkCableViewModel,
  MODIFIER  : Modifier = Modifier
)
{
  val isConnected       = VIEW_MODEL.isConnected.collectAsState()
  val partnerConnected  = VIEW_MODEL.isPartnerConnected.collectAsState()
  val sessionID         = VIEW_MODEL.sessionID.collectAsState()

  Column (
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    Image(
      painter            =
        if (!isConnected.value) painterResource(R.drawable.no_internet)
        else                    painterResource(R.drawable.linked_boys),
      modifier           = Modifier.fillMaxWidth(),
      contentDescription = null
    )

    if (!isConnected.value) CustomText("Connect to the internet to play with a buddy")
    else if (!partnerConnected.value)
    {
      Column(
        modifier            = Modifier
          .padding(16.dp)
          .border(4.dp, GameBoyColors.Green, RectangleShape)
          .wrapContentHeight()
          .fillMaxWidth(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
      )
      {
        CustomText(
          TEXT      = "Join a Room",
          FONT_SIZE = 42)

        val room = TextInp("Enter Room Number")
        CustomButton(
          { App.appModule.linkCable.joinSession(room)})
        { CustomText("Join") }
        Spacer(modifier = Modifier.size(8.dp))
      }

      Column(
        modifier            = Modifier
          .padding(16.dp)
          .border(4.dp, GameBoyColors.Green, RectangleShape)
          .wrapContentHeight()
          .fillMaxWidth(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
      )
      {
        CustomText(
          TEXT      = "Create a Room",
          FONT_SIZE = 42)

        if (sessionID.value != null)
        {
          CustomText(
            TEXT      = sessionID.value!!,
            MODIFIER  = Modifier.background(GameBoyColors.Green),
            COLOR     = GameBoyColors.DarkGreen,
          )
        }
        CustomButton(
          {
            App.appModule.linkCable.createSession()
          }
        )
        { CustomText("Create") }
        Spacer(modifier = Modifier.size(8.dp))
      }
    }
    else
    {
      Column(
        modifier            = Modifier
          .padding(16.dp)
          .border(4.dp, GameBoyColors.Green, RectangleShape)
          .wrapContentHeight()
          .fillMaxWidth(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
      )
      {
        CustomText(
          TEXT      = "Disconnect",
          FONT_SIZE = 42)

        if (sessionID.value != null)
        {
          CustomText(
            TEXT      = sessionID.value!!,
            MODIFIER  = Modifier.background(GameBoyColors.Green).fillMaxWidth(),
            COLOR     = GameBoyColors.DarkGreen,
          )
        }
        CustomButton(
          {
            App.appModule.linkCable.disconnect()
            VIEW_MODEL.resetSession()
          })
        {
          CustomText(
            "Disconnect",
            COLOR = GameBoyColors.Error)

        }
        Spacer(modifier = Modifier.size(8.dp))
      }
    }
  }
}