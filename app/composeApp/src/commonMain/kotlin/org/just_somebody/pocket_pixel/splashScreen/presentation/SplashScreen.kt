package org.just_somebody.pocket_pixel.splashScreen.presentation

import CustomButton
import TextInp
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.compose.rememberNavController
import kotlinx.coroutines.delay
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.Screens
import org.just_somebody.pocket_pixel.core.isLandscape
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import org.just_somebody.pocket_pixel.depInj.getNavController
import pocketpixel.composeapp.generated.resources.GameBoy
import pocketpixel.composeapp.generated.resources.Res

private var hasAutoLoginDone : Boolean = false;

@Composable
fun SplashScreen(
  MODIFIER     : Modifier         = Modifier,
  VIEW_MODEL   : SplashViewModel  = SplashViewModel()
)
{
  val state             =           VIEW_MODEL.state
  val navController     =           getNavController()
  var showLogin         by remember { mutableStateOf(false) }
  var hasAutoLoginDone  by remember { mutableStateOf(false) }
  var gamerTag          by remember { mutableStateOf("") }
  var password          by remember { mutableStateOf("") }

  LaunchedEffect(Unit)
  {
    delay(3000)
    showLogin = true
  }

  // - - - Auto-login logic (only once)
  LaunchedEffect(Unit)
  {
    if (!hasAutoLoginDone)
    {
      VIEW_MODEL.onAction(SplashActions.AutoLogin)
      hasAutoLoginDone = true
    }
  }

  Column(
    modifier              = MODIFIER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    horizontalAlignment   = Alignment.CenterHorizontally,
    verticalArrangement   = Arrangement.Center
  )
  {
    // - - - Logo
    Image(
      painter             = painterResource(Res.drawable.GameBoy),
      contentDescription  = null,
      modifier            = Modifier.size(256.dp)
    )

    // - - - Title
    Text(
      text                = "Pocket Pixel",
      color               = GameBoyColors.LightGreen,
      fontSize            = 72.sp,
      fontFamily          = PokeFontFamily(),
    )

    Spacer(modifier = Modifier.size(32.dp))

    // - - - Show login/register only after delay
    if (showLogin)
    {
      if (isLandscape())
      {
        Row ()
        {
          gamerTag = TextInp(PLACEHOLDER = "GamerTag")
          password = TextInp(PLACEHOLDER = "Password")
        }
        Row()
        {
          CustomButton(
            CONTENT   = { text("Login") },
            MODIFIER  = Modifier.width(256.dp),
            ON_CLICK  = { VIEW_MODEL.onAction(SplashActions.Login) }
          )
          CustomButton(
            CONTENT   = { text("Register") },
            MODIFIER  = Modifier.width(256.dp),
            ON_CLICK  = { VIEW_MODEL.onAction(SplashActions.Register) }
          )
        }
      }
      else
      {
        Column ()
        {
          gamerTag = TextInp(PLACEHOLDER = "GamerTag")
          password = TextInp(PLACEHOLDER = "Password")
        }
        Column ()
        {
          CustomButton(
            CONTENT   = { text("Login") },
            MODIFIER  = Modifier.width(256.dp),
            ON_CLICK  = { VIEW_MODEL.onAction(SplashActions.Login) }
          )
          CustomButton(
            CONTENT   = { text("Register") },
            MODIFIER  = Modifier.width(256.dp),
            ON_CLICK  = { VIEW_MODEL.onAction(SplashActions.Register) }
          )
        }
      }
      // - - - Guest button
      Text(
        modifier    = Modifier
          .padding(16.dp)
          .clickable { navController.navigate(Screens.MainScreen) },
        text        = "Guest",
        color       = GameBoyColors.LightGreen,
        fontSize    = 32.sp,
        fontFamily  = PokeFontFamily(),
        fontStyle   = FontStyle.Italic
      )
    }

    if (state.isLoginError)
    {
      Text(
        modifier    = Modifier.padding(16.dp),
        text        = "Failure, you are",
        color       = Color.Red,
        fontSize    = 32.sp,
        fontStyle   = FontStyle.Italic,
        fontFamily  = PokeFontFamily(),
      )
    }
  }
}

@Composable
fun text(TEXT : String)
{
  Text(
    modifier    = Modifier.padding(16.dp),
    text        = TEXT,
    color       = GameBoyColors.LightGreen,
    fontSize    = 32.sp,
    fontFamily  = PokeFontFamily(),
  )
}