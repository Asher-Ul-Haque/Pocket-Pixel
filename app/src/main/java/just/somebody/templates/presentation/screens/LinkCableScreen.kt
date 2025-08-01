package just.somebody.templates.presentation.screens

import android.content.ClipData
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
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.platform.ClipEntry
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.LinkCableViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.TextInp
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.launch
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.text.buildAnnotatedString


@Composable
fun LinkCableScreen(
  VIEW_MODEL: LinkCableViewModel,
  MODIFIER: Modifier = Modifier
) {
  val isNetworkAvailable  = VIEW_MODEL.isNetworkAvailable.collectAsState()
  val sessionID           = VIEW_MODEL.sessionID.collectAsState()
  val isConnectedToServer = VIEW_MODEL.isConnectedToServer.collectAsState()
  val isInSession         = VIEW_MODEL.isInSession.collectAsState()
  val isPartnerConnected  = VIEW_MODEL.isPartnerConnected.collectAsState()
  val scope = rememberCoroutineScope()

  // - - - Show a snackbar when network is lost
  LaunchedEffect(isNetworkAvailable.value)
  {
    if (isNetworkAvailable.value != NetworkStatus.Available)
    {
      SnackbarController.sendEvent(SnackbarEvent("Lost internet connectivity", null))
    }
  }

  Column(
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {

    // - - - Status Message
    if (isNetworkAvailable.value != NetworkStatus.Available)
    { CustomText("Connect to the internet to play with a buddy") }


    // - - - Show the appropriate image
    Image(
      painter =
        if (isNetworkAvailable.value != NetworkStatus.Available)  painterResource(R.drawable.no_internet)
        else                                                      painterResource(R.drawable.linked_boys),
      modifier =
        if (isNetworkAvailable.value == NetworkStatus.Available)  Modifier.fillMaxWidth()
        else                                                      Modifier.fillMaxSize(0.3f),
      contentDescription = null
    )

    Spacer(modifier = Modifier.size(16.dp))

    // - - - Session UI (Only when network is available)
    if (isNetworkAvailable.value == NetworkStatus.Available && isConnectedToServer.value)
    {
      when
      {
        !isInSession.value ->
          {
          // - - - Not in session → show create & join options
          ColumnBoxed("Join a Room")
          {
            val room = TextInp("Enter Room Number")
            CustomButton(ON_CLICK = { VIEW_MODEL.joinSession(room) })
            { CustomText("Join") }
          }

          ColumnBoxed("Create a Room")
          {
            CustomButton(ON_CLICK= { VIEW_MODEL.createSession() })
            { CustomText("Create") }
          }
        }

        isInSession.value ->
          {
          ColumnBoxed("Session Active")
          {
            sessionID.let {
              it.value?.let { it1 ->
                CustomText(
                  TEXT = it1,
                  MODIFIER = Modifier.background(GameBoyColors.Green),
                  COLOR = GameBoyColors.DarkGreen
                )

                val clipboard = LocalClipboard.current
                val sessionIdToCopy = sessionID.value ?: ""

                CustomButton(ON_CLICK =
                {
                  scope.launch()
                  {
                    val clipData = ClipData.newPlainText("Session ID", sessionIdToCopy)
                    clipboard.setClipEntry(ClipEntry(clipData))
                    SnackbarController.sendEvent(SnackbarEvent("Session ID copied!", null))
                  }
                })
                { CustomText("Copy") }
              }
            }

            if (isPartnerConnected.value) CustomText("Partner Connected!")
            else                          CustomText("Waiting for partner...")

            CustomButton(
              ON_CLICK = { VIEW_MODEL.disconnect() },
              COLOR    = GameBoyColors.Error)
            { CustomText("Disconnect") }
          }
        }
      }
    }
  }
}

// - - -UI helper
@Composable
private fun ColumnBoxed(title: String, content: @Composable () -> Unit) {
  Column(
    modifier = Modifier
      .padding(16.dp)
      .border(4.dp, GameBoyColors.Green, RectangleShape)
      .wrapContentHeight()
      .fillMaxWidth(),
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    CustomText(TEXT = title, FONT_SIZE = 42)
    Spacer(modifier = Modifier.size(8.dp))
    content()
    Spacer(modifier = Modifier.size(8.dp))
  }
}
