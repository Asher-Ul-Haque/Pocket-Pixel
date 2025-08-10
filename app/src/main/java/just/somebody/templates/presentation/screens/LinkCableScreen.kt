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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.platform.ClipEntry
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
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




@Composable
fun LinkCableScreen(
  VIEW_MODEL  : LinkCableViewModel,
  MODIFIER    : Modifier = Modifier,
  SHOW_IMAGES : Boolean = true
)
{
  val isNetworkAvailable    = VIEW_MODEL.isNetworkAvailable.collectAsState()
  val sessionID             = VIEW_MODEL.sessionID.collectAsState()
  val isConnectedToServer   = VIEW_MODEL.isConnectedToServer.collectAsState()
  val isInSession           = VIEW_MODEL.isInSession.collectAsState()
  val isPartnerConnected    = VIEW_MODEL.isPartnerConnected.collectAsState()
  val isSessionNotFound     = VIEW_MODEL.isSessionNotFound.collectAsState()
  val isSessionFull         = VIEW_MODEL.isSessionFull.collectAsState()
  val waitingForTransfer    = VIEW_MODEL.waitingForTransfer.collectAsState()

  val scope     = rememberCoroutineScope()
  val clipboard = LocalClipboard.current
  var showCPY   = remember { mutableStateOf(false) }


  // - - - Show a snackbar when network is lost
  LaunchedEffect(isNetworkAvailable.value)
  {
    if (isNetworkAvailable.value != NetworkStatus.Available)
    { SnackbarController.sendEvent(SnackbarEvent("Lost internet connectivity", null)) }
  }

  // - - - Show snackbars for session errors
  LaunchedEffect(isSessionNotFound.value)
  {
    if (isSessionNotFound.value)
    { SnackbarController.sendEvent(SnackbarEvent("Session not found. Please check the ID.", null)) }
  }

  LaunchedEffect(isSessionFull.value)
  {
    if (isSessionFull.value) { SnackbarController.sendEvent(SnackbarEvent("Session is full. Try another ID.", null)) }
  }

  Column(
    modifier            = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    verticalArrangement = Arrangement.Center,
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    // - - - Show the appropriate image
    if (SHOW_IMAGES)
    {
      Image(
        painter =
          if (isNetworkAvailable.value != NetworkStatus.Available)  painterResource(R.drawable.no_internet)
          else                                                      painterResource(R.drawable.linked_boys),
        modifier =
          if (isNetworkAvailable.value == NetworkStatus.Available)  Modifier.fillMaxWidth()
          else                                                      Modifier.fillMaxSize(0.3f),
        contentDescription = null
      )
    }

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
            CustomButton(ON_CLICK = { VIEW_MODEL.joinSession(room); showCPY.value = false })
            { CustomText("Join") }
          }

          ColumnBoxed("Create a Room")
          {
            CustomButton(ON_CLICK= { VIEW_MODEL.createSession(); showCPY.value = true })
            { CustomText("Create") }
          }
        }

        isInSession.value ->
        {
          ColumnBoxed("Session Active")
          {
            sessionID.let()
            {
              it.value?.let()
              { it1 ->
                CustomText(
                  TEXT      = it1,
                  MODIFIER  = Modifier.background(GameBoyColors.Green),
                  COLOR     = GameBoyColors.DarkGreen
                )


                if (showCPY.value && !isSessionFull.value)
                {
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
            }

            if (isPartnerConnected.value)
            {
              if (waitingForTransfer.value) CustomText("Waiting for partner to send data...", COLOR = GameBoyColors.LightGreen)
              else                          CustomText("Partner Connected!", COLOR = GameBoyColors.LightGreen)
            }
            else
            {
              CustomText("Waiting for partner...", COLOR = GameBoyColors.LightGreen)
            }

            CustomButton(
              ON_CLICK = { VIEW_MODEL.disconnect() },
              COLOR    = GameBoyColors.Error)
            { CustomText("Disconnect") }
          }
        }
      }
    }
    else
    {
      if (isNetworkAvailable.value != NetworkStatus.Available)
      {
        CustomText("Not connected to the internet")
      }
      else
      {
        CustomText("Connecting to Link Cable Club")
      }
    }
  }
}

// - - -UI helper
@Composable
private fun ColumnBoxed(TITLE : String, CONTENT : @Composable () -> Unit) 
{
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
    CustomText(TEXT = TITLE, FONT_SIZE = 42)
    Spacer(modifier = Modifier.size(4.dp))
    CONTENT()
    Spacer(modifier = Modifier.size(4.dp))
  }
}