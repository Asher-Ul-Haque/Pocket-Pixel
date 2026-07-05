package just.somebody.templates.presentation.screens

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.data.entities.AchievementEntity
import just.somebody.templates.presentation.viewModels.AchievementViewModel
import just.somebody.templates.presentation.viewModels.GroupedAchievements
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.DeviceSizePreviews
import java.text.SimpleDateFormat
import java.util.*

@Composable
fun AchievementScreen(VIEW_MODEL: AchievementViewModel)
{
  val settings              by VIEW_MODEL.settings.collectAsState()
  val groupedAchievements   by VIEW_MODEL.groupedAchievements.collectAsState()
  val loginError            by VIEW_MODEL.loginError.collectAsState()
  val isConnected           by App
    .appModule.hardwareManager
    .isConnectedToInternet
    .collectAsState(initial = NetworkStatus.Lost)

    Box(
      modifier = Modifier
        .fillMaxSize()
        .background(GameBoyColors.DarkGreen)
        .padding(16.dp))
    {
      if (isConnected != NetworkStatus.Available && settings.raToken.isEmpty())
      { NoInternetContent() }
      else if (settings.raToken.isEmpty())
      {
        LoginContent(
          LOGIN_ERROR = loginError,
          ON_LOGIN    = { user, token -> VIEW_MODEL.login(user, token) })
      }
      else
      {
        ProfileContent(
          USERNAME              = settings.raUsername,
          GROUPED_ACHIEVEMENTS  = groupedAchievements,
          ON_LOGOUT             = { VIEW_MODEL.logout() })
      }
    }
}

@Composable
fun NoInternetContent()
{
  Column(
    modifier            = Modifier.fillMaxSize(),
    horizontalAlignment = Alignment.CenterHorizontally,
    verticalArrangement = Arrangement.Center)
  {
    Icon(
      painter               = painterResource(R.drawable.no_internet),
      contentDescription    = null,
      modifier              = Modifier.size(80.dp))
    Spacer(Modifier.height(16.dp))
    CustomText(
      stringResource(R.string.NO_INTERNET),
      FONT_SIZE     = 14,
      COLOR         = GameBoyColors.LightGreen,
      TEXT_ALIGN    = TextAlign.Center,
      MODIFIER      = Modifier.padding(horizontal = 32.dp))
  }
}

@Composable
fun LoginContent(LOGIN_ERROR: String?, ON_LOGIN: (String, String) -> Unit)
{
  var username  by remember { mutableStateOf("") }
  var token     by remember { mutableStateOf("") }
  val uriHandler = LocalUriHandler.current

  LazyColumn(
    modifier            = Modifier.fillMaxSize(),
    horizontalAlignment = Alignment.CenterHorizontally,
    verticalArrangement = Arrangement.Center)
  {
    item()
    {
      Image(
        painter             = painterResource(R.drawable.winner),
        contentDescription  = null,
        modifier            = Modifier.size(64.dp))
      Spacer(Modifier.height(16.dp))
      CustomText(
        stringResource(R.string.RA),
        FONT_SIZE   = 24,
        COLOR       = GameBoyColors.LightGreen)
      Spacer(Modifier.height(32.dp))

      RAInput(
        LABEL           = stringResource(R.string.USERNAME),
        VALUE           = username,
        ON_VALUE_CHANGE = { username = it })
      Spacer(Modifier.height(16.dp))
      RAInput(
        LABEL           = stringResource(R.string.PASSWORD),
        VALUE           = token,
        ON_VALUE_CHANGE = { token = it },
        IS_PASSWORD     = true)

      if (LOGIN_ERROR != null)
      {
        Spacer(Modifier.height(8.dp))
        CustomText(
          " ${stringResource(R.string.LOGIN_ERROR)} : $LOGIN_ERROR",
          FONT_SIZE = 12,
          COLOR     = GameBoyColors.Error)
      }

      Spacer(Modifier.height(32.dp))
      CustomButton(
        ON_CLICK =
          {
            if (username.isNotEmpty() && token.isNotEmpty()) ON_LOGIN(username, token)
          },
        MODIFIER = Modifier.fillMaxWidth())
      { CustomText(stringResource(R.string.LOGIN), FONT_SIZE = 18) }

      Spacer(Modifier.height(16.dp))
      TextButton(
        onClick =
          { uriHandler.openUri("https://retroachievements.org/") })
      {
        CustomText(
          stringResource(R.string.HELP_TEXT),
          FONT_SIZE  = 12,
          COLOR      = GameBoyColors.MediumGreen,
          TEXT_ALIGN = TextAlign.Center)
      }
    }
  }
}

@Composable
fun ProfileContent(
  USERNAME              : String,
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  ON_LOGOUT             : () -> Unit)
{
  Column(modifier = Modifier.fillMaxSize())
  {
    Row(
      modifier              = Modifier.fillMaxWidth(),
      horizontalArrangement = Arrangement.SpaceBetween,
      verticalAlignment     = Alignment.CenterVertically)
    {
      Column()
        {
          CustomText(
            "${stringResource(R.string.USER)} $USERNAME",
            FONT_SIZE   = 18,
            COLOR       = GameBoyColors.LightGreen)
          val totalCount = GROUPED_ACHIEVEMENTS.sumOf { it.achievements.size }
          CustomText(
            "$totalCount ${stringResource(R.string.Achievements)}",
            FONT_SIZE   = 14,
            COLOR       = GameBoyColors.MediumGreen)
        }
    }

    Spacer(Modifier.height(16.dp))
    HorizontalDivider(color = GameBoyColors.MediumGreen)
    Spacer(Modifier.height(16.dp))

    Box(modifier = Modifier.weight(1f))
    {
      LazyColumn(
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.spacedBy(16.dp))
      {
        GROUPED_ACHIEVEMENTS.forEach()
        { grouped ->
          item()
          {
            CustomText(
              grouped.gameTitle,
              FONT_SIZE = 16,
              COLOR     = GameBoyColors.LightGreen)
            Spacer(Modifier.height(8.dp))
          }
          items(grouped.achievements)
          { achievement ->
            AchievementItem(achievement)
            Spacer(Modifier.height(8.dp))
          }
        }
      }
    }

    Spacer(Modifier.height(16.dp))
    CustomButton(
      ON_CLICK  = ON_LOGOUT,
      MODIFIER  = Modifier.fillMaxWidth(),
      COLOR     = GameBoyColors.Error)
    {
      Row(
        modifier                = Modifier
          .fillMaxWidth()
          .padding(horizontal = 12.dp),
        verticalAlignment       = Alignment.CenterVertically,
        horizontalArrangement   = Arrangement.Start)
      {
        Icon(
          painterResource(R.drawable.trash),
          null,
          tint      = GameBoyColors.DarkGreen,
          modifier  = Modifier.size(18.dp))
        Spacer(Modifier.width(12.dp))
        CustomText(stringResource(R.string.LOGOUT), FONT_SIZE = 14)
      }
    }
  }
}

@Composable
fun AchievementItem(ACHIEVEMENT: AchievementEntity)
{
  Row(
    modifier = Modifier
      .fillMaxWidth()
      .border(2.dp, GameBoyColors.Green)
      .background(GameBoyColors.MediumGreen.copy(alpha = 0.2f))
      .padding(4.dp),
    verticalAlignment = Alignment.CenterVertically)
  {
    AsyncImage(
      model                 = ACHIEVEMENT.badgeUrl,
      contentDescription    = null,
      modifier              = Modifier
        .size(96.dp)
        .background(GameBoyColors.DarkGreen)
        .border(1.dp, GameBoyColors.Green),
      placeholder = painterResource(R.drawable.winner))

    Spacer(Modifier.width(12.dp))

    Column(modifier = Modifier.weight(1f))
    {
      CustomText(
        ACHIEVEMENT.title,
        FONT_SIZE   = 14,
        COLOR       = GameBoyColors.LightGreen,
        MODIFIER    = Modifier.padding(0.dp))
      CustomText(
        ACHIEVEMENT.description,
        FONT_SIZE   = 10,
        COLOR       = GameBoyColors.Green,
        MODIFIER    = Modifier.padding(0.dp))
            
      val date = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date(ACHIEVEMENT.unlockDate))
      CustomText("${stringResource(R.string.Unlocked)} $date", FONT_SIZE = 9, COLOR = GameBoyColors.MediumGreen, MODIFIER = Modifier.padding(0.dp))
    }

    Column(
      horizontalAlignment   = Alignment.End,
      modifier              = Modifier.padding(end = 8.dp))
    {
      CustomText(
        "${ACHIEVEMENT.points}",
        FONT_SIZE   = 14,
        COLOR       = GameBoyColors.LightGreen,
        MODIFIER    = Modifier.padding(0.dp))
      if (ACHIEVEMENT.isHardcore)
      {
        CustomText(
          stringResource(R.string.HARDCORE),
          FONT_SIZE = 10,
          COLOR     = Color.Red,
          MODIFIER  = Modifier.padding(0.dp))
      }
    }
  }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun RAInput(
  LABEL             : String,
  VALUE             : String,
  ON_VALUE_CHANGE   : (String) -> Unit,
  IS_PASSWORD       : Boolean = false)
{
  Column(modifier = Modifier.fillMaxWidth())
  {
    CustomText(
      LABEL,
      FONT_SIZE = 14,
      COLOR     = GameBoyColors.LightGreen)
    TextField(
      value                     = VALUE,
      onValueChange             = ON_VALUE_CHANGE,
      modifier                  = Modifier
        .fillMaxWidth()
        .padding(top = 4.dp)
        .background(GameBoyColors.Green),
      colors                    = TextFieldDefaults.colors(
      focusedTextColor          = GameBoyColors.DarkGreen,
      unfocusedTextColor        = GameBoyColors.DarkGreen,
      focusedContainerColor     = Color.Transparent,
      unfocusedContainerColor   = Color.Transparent,
      cursorColor               = GameBoyColors.DarkGreen,
      focusedIndicatorColor     = Color.Transparent,
      unfocusedIndicatorColor   = Color.Transparent),
      singleLine                = true,
      visualTransformation      =
        if (IS_PASSWORD)    androidx.compose.ui.text.input.PasswordVisualTransformation()
        else                androidx.compose.ui.text.input.VisualTransformation.None,
      shape = RectangleShape)
    }
}

@DeviceSizePreviews
@Composable
fun AchievementScreenProfilePreview() {
    val mockAchievements = listOf(
        AchievementEntity(
          raId            = 1,
          raGameId        = 4,
          raGameTitle     = "Super Mario Land",
          gameId          = 1L,
          title           = "First Steps",
          description     = "Started your first game",
          points          = 5,
          badgeUrl        = "",
          unlockDate      = System.currentTimeMillis(),
          isUnlocked      = true,
          isHardcore      = false),
        AchievementEntity(
          raId            = 2,
          raGameId        = 4,
          raGameTitle     = "Super Mario Land",
          gameId          = 1L,
          title           = "Hardcore Master",
          description     = "Beat the first boss in HC",
          points          = 25,
          badgeUrl        = "",
          unlockDate      = System.currentTimeMillis(),
          isUnlocked      = true,
          isHardcore      = true)
    )
    val mockGrouped = listOf(GroupedAchievements("Super Mario Land", mockAchievements))
    Box(modifier = Modifier.fillMaxSize().background(GameBoyColors.DarkGreen)) {
        ProfileContent(
            USERNAME = "Gamer123",
            GROUPED_ACHIEVEMENTS = mockGrouped,
            ON_LOGOUT = {}
        )
    }
}

@DeviceSizePreviews
@Composable
fun AchievementScreenLoginPreview() {
    Box(modifier = Modifier.fillMaxSize().background(GameBoyColors.DarkGreen)) {
        LoginContent(LOGIN_ERROR = null, ON_LOGIN = { _, _ -> })
    }
}
