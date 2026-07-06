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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AchievementScreen(VIEW_MODEL: AchievementViewModel)
{
  val settings              by VIEW_MODEL.settings.collectAsState()
  val groupedAchievements   by VIEW_MODEL.groupedAchievements.collectAsState()
  val loginError            by VIEW_MODEL.loginError.collectAsState()
  val isLoading             by VIEW_MODEL.isLoading.collectAsState()
  val isDownloading         by VIEW_MODEL.isDownloading.collectAsState()
  val isConnected           by App
    .appModule.hardwareManager
    .isConnectedToInternet
    .collectAsState(initial = NetworkStatus.Lost)

  var selectedAchievement by remember { mutableStateOf<AchievementEntity?>(null) }
  val sheetState          = rememberModalBottomSheetState()
  var showBottomSheet     by remember { mutableStateOf(false) }

    Box(
      modifier = Modifier
        .fillMaxSize()
        .background(GameBoyColors.DarkGreen)
        .padding(16.dp))
    {
      if (isLoading)
      { LoadingContent() }
      else if (isConnected != NetworkStatus.Available && settings.raToken.isEmpty())
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
          IS_DOWNLOADING        = isDownloading,
          ON_ACHIEVEMENT_CLICK  =
            {
              selectedAchievement = it
              showBottomSheet = true
            })
      }
      
      if (showBottomSheet && selectedAchievement != null)
      {
        ModalBottomSheet(
          onDismissRequest  = { showBottomSheet = false },
          sheetState        = sheetState,
          containerColor    = GameBoyColors.DarkGreen,
          contentColor      = GameBoyColors.LightGreen,
          shape             = RectangleShape,
          dragHandle        = { BottomSheetDefaults.DragHandle(color = GameBoyColors.Green) }
          )
        {
          BadgeDetailContent(
            ACHIEVEMENT = selectedAchievement!!,
            ON_DISMISS = { showBottomSheet = false })
        }
      }
    }
}

@Composable
fun LoadingContent()
{
  Column(
    modifier            = Modifier.fillMaxSize(),
    horizontalAlignment = Alignment.CenterHorizontally,
    verticalArrangement = Arrangement.Center)
  {
    CircularProgressIndicator(color = GameBoyColors.LightGreen)
    Spacer(Modifier.height(16.dp))
    CustomText(stringResource(R.string.SYNC), COLOR = GameBoyColors.LightGreen)
  }
}

@Composable
fun BadgeDetailContent(ACHIEVEMENT: AchievementEntity, ON_DISMISS: () -> Unit)
{
  LazyColumn(
      modifier = Modifier
        .fillMaxWidth()
        .padding(16.dp),
      horizontalAlignment = Alignment.CenterHorizontally)
  {
    item ()
    {
      val grayscale = ColorMatrix().apply { setToSaturation(0f) }
      val model     =
        if (ACHIEVEMENT.badgeUrl.isEmpty()) R.drawable.winner
        else                                ACHIEVEMENT.badgeUrl

      AsyncImage(
        model               = model,
        contentDescription  = null,
        modifier            = Modifier
          .size(144.dp)
          .background(GameBoyColors.DarkGreen)
          .border(2.dp, GameBoyColors.LightGreen),
        placeholder         = painterResource(R.drawable.winner),
        error               = painterResource(R.drawable.winner),
        colorFilter         =
          if (!ACHIEVEMENT.isUnlocked)  ColorFilter.colorMatrix(grayscale)
          else                          null,
        alpha =
          if (!ACHIEVEMENT.isUnlocked)  0.5f
          else                          1.0f)
            
      Spacer(Modifier.height(16.dp))
      CustomText(
        ACHIEVEMENT.title,
        FONT_SIZE   = 20,
        COLOR       = GameBoyColors.LightGreen,
        MODIFIER    = Modifier.fillMaxWidth(),
        TEXT_ALIGN  = TextAlign.Center)

      Spacer(Modifier.height(8.dp))
      CustomText(
        ACHIEVEMENT.description,
        FONT_SIZE   = 14,
        COLOR       = GameBoyColors.Green,
        TEXT_ALIGN  = TextAlign.Center,
        MODIFIER    = Modifier.fillMaxWidth())

      Spacer(Modifier.height(16.dp))
      HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 2.dp)
      Spacer(Modifier.height(16.dp))
    }

    item { DetailRow(stringResource(R.string.POINTS), "${ACHIEVEMENT.points}") }
        
    val typeStr = when (ACHIEVEMENT.type)
    {
      1     -> "Missable"
      2     -> "Progression"
      3     -> "Win Condition"
      else  -> "Standard"
    }
    item { DetailRow(stringResource(R.string.TYPE), typeStr) }
        
    if (ACHIEVEMENT.isUnlocked)
    {
      val date = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date(ACHIEVEMENT.unlockDate))
      item { DetailRow(stringResource(R.string.Unlocked), date) }
      item()
      {
        DetailRow(
          stringResource(R.string.MODE),
          if (ACHIEVEMENT.isHardcore) stringResource(R.string.HARD)
          else                        stringResource(R.string.SOFT))
      }
    }
    else
    {
      item { DetailRow(stringResource(R.string.STATUS), stringResource(R.string.LOCKED)) }
      if (ACHIEVEMENT.measuredProgress.isNotEmpty())
      {
        item { DetailRow(stringResource(R.string.PROGRESS), ACHIEVEMENT.measuredProgress) }
      }
    }
  }
}

@Composable
fun DetailRow(LABEL: String, VALUE: String)
{
  Row(
    modifier              = Modifier
      .fillMaxWidth()
      .padding(vertical = 4.dp),
    horizontalArrangement = Arrangement.SpaceBetween)
  {
    CustomText(
      LABEL,
      FONT_SIZE = 12,
      COLOR     = GameBoyColors.MediumGreen,
      MODIFIER  = Modifier.padding(0.dp))
    CustomText(
      VALUE,
      FONT_SIZE = 12,
      COLOR     = GameBoyColors.LightGreen,
      MODIFIER  = Modifier.padding(0.dp))
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
  IS_DOWNLOADING        : Boolean,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  val isLandscape = App.appModule.isLandscape()

  if (isLandscape)
  {
    LandscapeProfileContent(
      USERNAME,
      AVATAR_URL,
      TOTAL_POINTS,
      HARDCORE_POINTS,
      GROUPED_ACHIEVEMENTS,
      IS_DOWNLOADING,
      ON_ACHIEVEMENT_CLICK
    )
  }
  else
  {
    PortraitProfileContent(
      USERNAME,
      AVATAR_URL,
      TOTAL_POINTS,
      HARDCORE_POINTS,
      GROUPED_ACHIEVEMENTS,
      IS_DOWNLOADING,
      ON_ACHIEVEMENT_CLICK
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
  IS_DOWNLOADING        : Boolean,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  Column(modifier = Modifier.fillMaxSize())
  {
    val totalUnlocked = GROUPED_ACHIEVEMENTS.sumOf { it.achievements.count { a -> a.isUnlocked } }
    HeaderSection(USERNAME, AVATAR_URL, TOTAL_POINTS, HARDCORE_POINTS, totalUnlocked)

    Spacer(Modifier.height(16.dp))
    HorizontalDivider(color = GameBoyColors.MediumGreen)
    Spacer(Modifier.height(16.dp))

    Box(modifier = Modifier.weight(1f))
    {
      AchievementList(
        GROUPED_ACHIEVEMENTS,
        IS_DOWNLOADING,
        ON_ACHIEVEMENT_CLICK)
    }
  }
}

@Composable
fun LandscapeProfileContent(
  USERNAME              : String,
  AVATAR_URL            : String,
  TOTAL_POINTS          : Int,
  HARDCORE_POINTS       : Int,
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  IS_DOWNLOADING        : Boolean,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  Row(
    modifier              = Modifier.fillMaxSize(),
    horizontalArrangement = Arrangement.spacedBy(16.dp))
  {
    // - - - Sidebar with profile
    Column(
      modifier = Modifier
        .width(220.dp)
        .fillMaxHeight()
        .background(GameBoyColors.MediumGreen.copy(alpha = 0.1f))
        .padding(8.dp),
      horizontalAlignment = Alignment.CenterHorizontally)
    {
      AsyncImage(
        model               = AVATAR_URL,
        contentDescription  = null,
        modifier            = Modifier
          .size(80.dp)
          .background(GameBoyColors.DarkGreen)
          .border(2.dp, GameBoyColors.LightGreen),
        placeholder = painterResource(R.drawable.winner))
      Spacer(Modifier.height(8.dp))
      CustomText(
        USERNAME,
        FONT_SIZE   = 16,
        COLOR       = GameBoyColors.LightGreen,
        TEXT_ALIGN  = TextAlign.Center,
        MODIFIER    = Modifier.padding(0.dp))
      
      val totalUnlocked = GROUPED_ACHIEVEMENTS.sumOf { it.achievements.count { a -> a.isUnlocked } }
      CustomText(
        "$totalUnlocked ${stringResource(R.string.ACHIEVEMENTS)}",
        FONT_SIZE   = 12,
        COLOR       = GameBoyColors.MediumGreen,
        TEXT_ALIGN  = TextAlign.Center,
        MODIFIER    = Modifier.padding(top = 4.dp))
      
      CustomText(
        "$TOTAL_POINTS pts ($HARDCORE_POINTS HC)",
        FONT_SIZE   = 11,
        COLOR       = GameBoyColors.LightGreen,
        TEXT_ALIGN  = TextAlign.Center,
        MODIFIER    = Modifier.padding(top = 8.dp))
    }

    // - - - Main content area
    Column(modifier = Modifier.weight(1f))
    {
      AchievementList(
        GROUPED_ACHIEVEMENTS,
        IS_DOWNLOADING,
        ON_ACHIEVEMENT_CLICK)
    }
  }
}

@Composable
fun HeaderSection(
  USERNAME        : String,
  AVATAR_URL      : String,
  TOTAL_POINTS    : Int,
  HARDCORE_POINTS : Int,
  TOTAL_UNLOCKED  : Int)
{
  Row(
    modifier              = Modifier.fillMaxWidth(),
    horizontalArrangement = Arrangement.SpaceBetween,
    verticalAlignment     = Alignment.CenterVertically)
  {
    Row(verticalAlignment = Alignment.CenterVertically)
    {
      AsyncImage(
        model               = AVATAR_URL,
        contentDescription  = null,
        modifier            = Modifier
          .size(48.dp)
          .background(GameBoyColors.DarkGreen)
          .border(2.dp, GameBoyColors.LightGreen),
        placeholder = painterResource(R.drawable.winner))
      Spacer(Modifier.width(12.dp))
      Column()
      {
        CustomText(
          USERNAME,
          FONT_SIZE   = 18,
          COLOR       = GameBoyColors.LightGreen,
          MODIFIER = Modifier.padding(0.dp))
        CustomText(
          "${stringResource(R.string.POINTS)}: $TOTAL_POINTS ($HARDCORE_POINTS HC)",
          FONT_SIZE   = 12,
          COLOR       = GameBoyColors.MediumGreen,
          MODIFIER = Modifier.padding(0.dp))
      }
    }
    
    CustomText(
      "$TOTAL_UNLOCKED ${stringResource(R.string.Achievements)}",
      FONT_SIZE   = 14,
      COLOR       = GameBoyColors.MediumGreen)
  }
}

@Composable
fun AchievementList(
  GROUPED_ACHIEVEMENTS  : List<GroupedAchievements>,
  IS_DOWNLOADING        : Boolean,
  ON_ACHIEVEMENT_CLICK  : (AchievementEntity) -> Unit)
{
  if (GROUPED_ACHIEVEMENTS.isEmpty() && !IS_DOWNLOADING)
  {
    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
      CustomText(
        stringResource(R.string.PLAY_TO_LOAD),
        COLOR       = GameBoyColors.MediumGreen,
        TEXT_ALIGN  = TextAlign.Center
      )
    }
  }
  else
  {
    LazyColumn(
      modifier            = Modifier.fillMaxSize(),
      verticalArrangement = Arrangement.spacedBy(16.dp))
    {
      GROUPED_ACHIEVEMENTS.forEach()
      { grouped ->
        item()
        {
          val unlockedInGame  = grouped.achievements.count { it.isUnlocked }
          val isMastered      = grouped.achievements.isNotEmpty() && unlockedInGame == grouped.achievements.size
          
          Column(
            modifier = Modifier
              .fillMaxWidth()
              .background(GameBoyColors.Green.copy(alpha = 0.1f))
              .border(1.dp, GameBoyColors.Green.copy(alpha = 0.2f))
              .padding(8.dp))
          {
            Row(verticalAlignment = Alignment.CenterVertically)
            {
              if (isMastered)
              {
                Icon(
                  painter             = painterResource(R.drawable.winner),
                  contentDescription  = null,
                  tint                = GameBoyColors.LightGreen,
                  modifier            = Modifier.size(16.dp))
                Spacer(Modifier.width(8.dp))
              }
              CustomText(
                grouped.gameTitle,
                FONT_SIZE = 16,
                COLOR     = GameBoyColors.LightGreen,
                MODIFIER  = Modifier.weight(1f).padding(0.dp))
                
              CustomText(
                "$unlockedInGame/${grouped.achievements.size}",
                FONT_SIZE = 12,
                COLOR = GameBoyColors.MediumGreen,
                MODIFIER = Modifier.padding(0.dp))
            }
            Spacer(Modifier.height(12.dp))
            
            LazyRow(
              horizontalArrangement = Arrangement.spacedBy(12.dp),
              contentPadding        = PaddingValues(horizontal = 4.dp))
            {
              items(grouped.achievements) { achievement -> AchievementBadgeItem(achievement, ON_ACHIEVEMENT_CLICK) }
            }
          }
        }
      }

      if (IS_DOWNLOADING)
      {
        item()
        {
          Box(
            modifier = Modifier
              .fillMaxWidth()
              .padding(16.dp),
            contentAlignment = Alignment.Center)
          {
            CircularProgressIndicator(
              color     = GameBoyColors.LightGreen,
              modifier  = Modifier.size(32.dp))
          }
        }
      }
    }
  }
}

@Composable
fun AchievementBadgeItem(ACHIEVEMENT: AchievementEntity, ON_CLICK: (AchievementEntity) -> Unit)
{
  val grayscale = ColorMatrix().apply { setToSaturation(0f) }
  val model     =
    if (ACHIEVEMENT.badgeUrl.isEmpty()) R.drawable.winner
    else                                ACHIEVEMENT.badgeUrl
    
  AsyncImage(
    model                 = model,
    contentDescription    = null,
    modifier              = Modifier
      .size(96.dp)
      .background(GameBoyColors.DarkGreen)
      .border(2.dp,
              if (ACHIEVEMENT.isUnlocked) GameBoyColors.LightGreen
              else                        GameBoyColors.MediumGreen)
      .clickable { ON_CLICK(ACHIEVEMENT) },
    placeholder = painterResource(R.drawable.winner),
    error       = painterResource(R.drawable.winner),
    colorFilter =
      if (!ACHIEVEMENT.isUnlocked) ColorFilter.colorMatrix(grayscale)
      else                         null,
    alpha       =
      if (!ACHIEVEMENT.isUnlocked)  0.5f
      else                          1.0f)
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
            IS_DOWNLOADING = false,
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
