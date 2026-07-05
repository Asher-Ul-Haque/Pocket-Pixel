package just.somebody.templates.presentation.screens

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ColorMatrix
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
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
import just.somebody.templates.ui.theme.MinecraftFontFamily
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

  var selectedAchievement by remember { mutableStateOf<AchievementEntity?>(null) }

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
          AVATAR_URL            = settings.raAvatarUrl,
          TOTAL_POINTS          = settings.raTotalPoints,
          HARDCORE_POINTS       = settings.raTotalHardcorePoints,
          GROUPED_ACHIEVEMENTS  = groupedAchievements,
          ON_RESYNC             = { VIEW_MODEL.resync() },
          ON_LOGOUT             = { VIEW_MODEL.logout() },
          ON_ACHIEVEMENT_CLICK  = { selectedAchievement = it })
      }
      
      selectedAchievement?.let { achievement ->
          BadgeDetailPanel(
              ACHIEVEMENT = achievement,
              ON_DISMISS = { selectedAchievement = null }
          )
      }
    }
}

@Composable
fun BadgeDetailPanel(ACHIEVEMENT: AchievementEntity, ON_DISMISS: () -> Unit) {
    Dialog(
        onDismissRequest = ON_DISMISS,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(Color.Black.copy(alpha = 0.7f))
                .clickable { ON_DISMISS() },
            contentAlignment = Alignment.Center
        ) {
            Column(
                modifier = Modifier
                    .width(if (App.appModule.isLandscape()) 500.dp else 320.dp)
                    .background(GameBoyColors.Green)
                    .border(4.dp, GameBoyColors.DarkGreen)
                    .padding(16.dp)
                    .clickable(enabled = false) { }
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.Center
                ) {
                    val grayscale = ColorMatrix().apply { setToSaturation(0f) }
                    AsyncImage(
                        model = ACHIEVEMENT.badgeUrl,
                        contentDescription = null,
                        modifier = Modifier
                            .size(128.dp)
                            .background(GameBoyColors.DarkGreen)
                            .border(2.dp, GameBoyColors.DarkGreen),
                        placeholder = painterResource(R.drawable.winner),
                        colorFilter = if (!ACHIEVEMENT.isUnlocked) ColorFilter.colorMatrix(grayscale) else null,
                        alpha = if (!ACHIEVEMENT.isUnlocked) 0.5f else 1.0f
                    )
                }

                Spacer(Modifier.height(16.dp))
                CustomText(
                    ACHIEVEMENT.title,
                    FONT_SIZE = 18,
                    COLOR = GameBoyColors.DarkGreen,
                    MODIFIER = Modifier.fillMaxWidth(),
                    TEXT_ALIGN = TextAlign.Center
                )
                Spacer(Modifier.height(8.dp))
                CustomText(
                    ACHIEVEMENT.description,
                    FONT_SIZE = 12,
                    COLOR = GameBoyColors.DarkGreen,
                    TEXT_ALIGN = TextAlign.Center,
                    MODIFIER = Modifier.fillMaxWidth()
                )

                Spacer(Modifier.height(16.dp))
                HorizontalDivider(color = GameBoyColors.DarkGreen, thickness = 2.dp)
                Spacer(Modifier.height(16.dp))

                DetailRow("Points", "${ACHIEVEMENT.points}")
                DetailRow("Rarity", "${"%.1f".format(ACHIEVEMENT.rarity)}%")
                
                if (ACHIEVEMENT.isUnlocked) {
                    val date = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date(ACHIEVEMENT.unlockDate))
                    DetailRow("Unlocked", date)
                    DetailRow("Mode", if (ACHIEVEMENT.isHardcore) "Hardcore" else "Softcore")
                } else {
                    DetailRow("Status", "Locked")
                    if (ACHIEVEMENT.measuredProgress.isNotEmpty()) {
                        DetailRow("Progress", ACHIEVEMENT.measuredProgress)
                    }
                }

                Spacer(Modifier.height(24.dp))
                CustomButton(
                    ON_CLICK = ON_DISMISS,
                    MODIFIER = Modifier.fillMaxWidth(),
                    COLOR = GameBoyColors.DarkGreen
                ) {
                    CustomText("Back", COLOR = GameBoyColors.LightGreen)
                }
            }
        }
    }
}

@Composable
fun DetailRow(LABEL: String, VALUE: String) {
    Row(
        modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        CustomText(LABEL, FONT_SIZE = 12, COLOR = GameBoyColors.DarkGreen, MODIFIER = Modifier.padding(0.dp))
        CustomText(VALUE, FONT_SIZE = 12, COLOR = GameBoyColors.DarkGreen, MODIFIER = Modifier.padding(0.dp))
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
  AVATAR_URL            : String,
  TOTAL_POINTS          : Int,
  HARDCORE_POINTS       : Int,
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  ON_RESYNC             : () -> Unit,
  ON_LOGOUT             : () -> Unit,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  val isLandscape = App.appModule.isLandscape()

  if (isLandscape) {
    LandscapeProfileContent(
      USERNAME, AVATAR_URL, TOTAL_POINTS, HARDCORE_POINTS,
      GROUPED_ACHIEVEMENTS, ON_RESYNC, ON_LOGOUT, ON_ACHIEVEMENT_CLICK
    )
  } else {
    PortraitProfileContent(
      USERNAME, AVATAR_URL, TOTAL_POINTS, HARDCORE_POINTS,
      GROUPED_ACHIEVEMENTS, ON_RESYNC, ON_LOGOUT, ON_ACHIEVEMENT_CLICK
    )
  }
}

@Composable
fun PortraitProfileContent(
  USERNAME              : String,
  AVATAR_URL            : String,
  TOTAL_POINTS          : Int,
  HARDCORE_POINTS       : Int,
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  ON_RESYNC             : () -> Unit,
  ON_LOGOUT             : () -> Unit,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  Column(modifier = Modifier.fillMaxSize())
  {
    HeaderSection(USERNAME, AVATAR_URL, TOTAL_POINTS, HARDCORE_POINTS, GROUPED_ACHIEVEMENTS.sumOf { it.achievements.size })

    Spacer(Modifier.height(16.dp))
    HorizontalDivider(color = GameBoyColors.MediumGreen)
    Spacer(Modifier.height(16.dp))

    Box(modifier = Modifier.weight(1f))
    {
      AchievementList(GROUPED_ACHIEVEMENTS, ON_ACHIEVEMENT_CLICK)
    }

    Spacer(Modifier.height(16.dp))
    ActionButtons(ON_RESYNC, ON_LOGOUT)
  }
}

@Composable
fun LandscapeProfileContent(
  USERNAME              : String,
  AVATAR_URL            : String,
  TOTAL_POINTS          : Int,
  HARDCORE_POINTS       : Int,
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  ON_RESYNC             : () -> Unit,
  ON_LOGOUT             : () -> Unit,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  Row(modifier = Modifier.fillMaxSize(), horizontalArrangement = Arrangement.spacedBy(16.dp))
  {
    // Sidebar with profile and actions
    Column(
      modifier = Modifier
        .width(220.dp)
        .fillMaxHeight()
        .background(GameBoyColors.MediumGreen.copy(alpha = 0.1f))
        .padding(8.dp),
      horizontalAlignment = Alignment.CenterHorizontally
    ) {
      AsyncImage(
        model = AVATAR_URL,
        contentDescription = null,
        modifier = Modifier
          .size(80.dp)
          .background(GameBoyColors.DarkGreen)
          .border(2.dp, GameBoyColors.LightGreen),
        placeholder = painterResource(R.drawable.winner)
      )
      Spacer(Modifier.height(8.dp))
      CustomText(USERNAME, FONT_SIZE = 16, COLOR = GameBoyColors.LightGreen, TEXT_ALIGN = TextAlign.Center)
      CustomText("$TOTAL_POINTS pts", FONT_SIZE = 12, COLOR = GameBoyColors.LightGreen, TEXT_ALIGN = TextAlign.Center)
      CustomText("($HARDCORE_POINTS HC)", FONT_SIZE = 10, COLOR = GameBoyColors.MediumGreen, TEXT_ALIGN = TextAlign.Center)
      
      Spacer(Modifier.weight(1f))
      
      ActionButtons(ON_RESYNC, ON_LOGOUT, isVertical = true)
    }

    // Main content area
    Column(modifier = Modifier.weight(1f)) {
      CustomText(
        "${GROUPED_ACHIEVEMENTS.sumOf { it.achievements.size }} Total Achievements",
        FONT_SIZE = 14,
        COLOR = GameBoyColors.MediumGreen
      )
      Spacer(Modifier.height(8.dp))
      AchievementList(GROUPED_ACHIEVEMENTS, ON_ACHIEVEMENT_CLICK)
    }
  }
}

@Composable
fun HeaderSection(USERNAME: String, AVATAR_URL: String, TOTAL_POINTS: Int, HARDCORE_POINTS: Int, TOTAL_COUNT: Int) {
  Row(
    modifier              = Modifier.fillMaxWidth(),
    horizontalArrangement = Arrangement.SpaceBetween,
    verticalAlignment     = Alignment.CenterVertically)
  {
    Row(verticalAlignment = Alignment.CenterVertically) {
        AsyncImage(
            model = AVATAR_URL,
            contentDescription = null,
            modifier = Modifier
                .size(48.dp)
                .background(GameBoyColors.DarkGreen)
                .border(2.dp, GameBoyColors.LightGreen),
            placeholder = painterResource(R.drawable.winner)
        )
        Spacer(Modifier.width(12.dp))
        Column()
        {
          CustomText(
            USERNAME,
            FONT_SIZE   = 18,
            COLOR       = GameBoyColors.LightGreen,
            MODIFIER = Modifier.padding(0.dp))
          CustomText(
            "Points: $TOTAL_POINTS ($HARDCORE_POINTS HC)",
            FONT_SIZE   = 12,
            COLOR       = GameBoyColors.MediumGreen,
            MODIFIER = Modifier.padding(0.dp))
        }
    }
    
    CustomText(
      "$TOTAL_COUNT ${stringResource(R.string.Achievements)}",
      FONT_SIZE   = 14,
      COLOR       = GameBoyColors.MediumGreen)
  }
}

@Composable
fun AchievementList(GROUPED_ACHIEVEMENTS: List<GroupedAchievements>, ON_ACHIEVEMENT_CLICK: (AchievementEntity) -> Unit) {
  LazyColumn(
    modifier            = Modifier.fillMaxSize(),
    verticalArrangement = Arrangement.spacedBy(24.dp))
  {
    GROUPED_ACHIEVEMENTS.forEach()
    { grouped ->
      item()
      {
        val isMastered = grouped.achievements.isNotEmpty() && grouped.achievements.all { it.isUnlocked }
        Row(verticalAlignment = Alignment.CenterVertically) {
            if (isMastered) {
                Icon(
                    painter = painterResource(R.drawable.winner),
                    contentDescription = null,
                    tint = GameBoyColors.LightGreen,
                    modifier = Modifier.size(16.dp)
                )
                Spacer(Modifier.width(8.dp))
            }
            CustomText(
              grouped.gameTitle,
              FONT_SIZE = 16,
              COLOR     = GameBoyColors.LightGreen)
        }
        Spacer(Modifier.height(8.dp))
        
        LazyRow(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            contentPadding = PaddingValues(horizontal = 4.dp)
        ) {
            items(grouped.achievements) { achievement ->
                AchievementBadgeItem(achievement, ON_ACHIEVEMENT_CLICK)
            }
        }
      }
    }
  }
}

@Composable
fun ActionButtons(ON_RESYNC: () -> Unit, ON_LOGOUT: () -> Unit, isVertical: Boolean = false) {
  if (isVertical) {
    Column(modifier = Modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        CustomButton(
          ON_CLICK  = ON_RESYNC,
          MODIFIER  = Modifier.fillMaxWidth(),
          COLOR     = GameBoyColors.Green)
        {
          Row(
            verticalAlignment       = Alignment.CenterVertically,
            horizontalArrangement   = Arrangement.Center)
          {
            Icon(
              painterResource(R.drawable.settings),
              null,
              tint      = GameBoyColors.DarkGreen,
              modifier  = Modifier.size(18.dp))
            Spacer(Modifier.width(8.dp))
            CustomText("Resync", FONT_SIZE = 14, COLOR = GameBoyColors.DarkGreen, MODIFIER = Modifier.padding(0.dp))
          }
        }

        CustomButton(
          ON_CLICK  = ON_LOGOUT,
          MODIFIER  = Modifier.fillMaxWidth(),
          COLOR     = GameBoyColors.Error)
        {
          Row(
            verticalAlignment       = Alignment.CenterVertically,
            horizontalArrangement   = Arrangement.Center)
          {
            Icon(
              painterResource(R.drawable.trash),
              null,
              tint      = GameBoyColors.DarkGreen,
              modifier  = Modifier.size(18.dp))
            Spacer(Modifier.width(8.dp))
            CustomText(stringResource(R.string.LOGOUT), FONT_SIZE = 14, MODIFIER = Modifier.padding(0.dp))
          }
        }
    }
  } else {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
        CustomButton(
          ON_CLICK  = ON_RESYNC,
          MODIFIER  = Modifier.weight(1f),
          COLOR     = GameBoyColors.Green)
        {
          Row(
            verticalAlignment       = Alignment.CenterVertically,
            horizontalArrangement   = Arrangement.Center)
          {
            Icon(
              painterResource(R.drawable.settings),
              null,
              tint      = GameBoyColors.DarkGreen,
              modifier  = Modifier.size(18.dp))
            Spacer(Modifier.width(8.dp))
            CustomText("Resync", FONT_SIZE = 14, COLOR = GameBoyColors.DarkGreen, MODIFIER = Modifier.padding(0.dp))
          }
        }

        CustomButton(
          ON_CLICK  = ON_LOGOUT,
          MODIFIER  = Modifier.weight(1f),
          COLOR     = GameBoyColors.Error)
        {
          Row(
            verticalAlignment       = Alignment.CenterVertically,
            horizontalArrangement   = Arrangement.Center)
          {
            Icon(
              painterResource(R.drawable.trash),
              null,
              tint      = GameBoyColors.DarkGreen,
              modifier  = Modifier.size(18.dp))
            Spacer(Modifier.width(8.dp))
            CustomText(stringResource(R.string.LOGOUT), FONT_SIZE = 14, MODIFIER = Modifier.padding(0.dp))
          }
        }
    }
  }
}

@Composable
fun AchievementBadgeItem(ACHIEVEMENT: AchievementEntity, ON_CLICK: (AchievementEntity) -> Unit) {
    val grayscale = ColorMatrix().apply { setToSaturation(0f) }
    
    AsyncImage(
      model                 = ACHIEVEMENT.badgeUrl,
      contentDescription    = null,
      modifier              = Modifier
        .size(64.dp)
        .background(GameBoyColors.DarkGreen)
        .border(2.dp, if (ACHIEVEMENT.isUnlocked) GameBoyColors.LightGreen else GameBoyColors.MediumGreen)
        .clickable { ON_CLICK(ACHIEVEMENT) },
      placeholder = painterResource(R.drawable.winner),
      colorFilter = if (!ACHIEVEMENT.isUnlocked) ColorFilter.colorMatrix(grayscale) else null,
      alpha = if (!ACHIEVEMENT.isUnlocked) 0.5f else 1.0f
    )
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
            AVATAR_URL = "",
            TOTAL_POINTS = 30,
            HARDCORE_POINTS = 25,
            GROUPED_ACHIEVEMENTS = mockGrouped,
            ON_RESYNC = {},
            ON_LOGOUT = {},
            ON_ACHIEVEMENT_CLICK = {}
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
