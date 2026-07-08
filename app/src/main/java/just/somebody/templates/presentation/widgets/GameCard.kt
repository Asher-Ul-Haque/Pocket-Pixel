package just.somebody.templates.presentation.widgets

import androidx.compose.animation.core.animateDpAsState
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.core.*
import androidx.compose.animation.fadeIn
import androidx.compose.animation.expandHorizontally
import androidx.compose.animation.expandVertically
import androidx.compose.foundation.background
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.grid.LazyHorizontalGrid
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil.compose.AsyncImage
import coil.request.ImageRequest
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.models.Game
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flowOf

/**
 * A stylized grid or row item representing an individual game cartridge inside the dashboard list view.
 */
@Composable
fun GameCard(
  GAME          : Game,
  IMAGE_URL     : (Game) -> Flow<String?>,
  ON_CLICK      : (Game) -> Unit,
  ON_LONG_PRESS : (Game) -> Unit,
  BIG           : Boolean)
{
  val SCALE = if (BIG) 1f else 0.95f

  val initials = remember(GAME.title) {
    GAME.title
      .split(Regex("\\s+"))
      .take(3)
      .mapNotNull { word -> word.firstOrNull { it.isLetterOrDigit() } }
      .joinToString("") { it.uppercaseChar().toString() }
  }

  val cardWidth = if (BIG) 200.dp * SCALE else 140.dp * SCALE
  val titleFont = if (BIG) 16 else 12

  val interactionSource = remember { MutableInteractionSource() }
  val isPressed by interactionSource.collectIsPressedAsState()
  
  val cardScale by animateFloatAsState(
    targetValue = if (isPressed) 1.05f else 1f,
    animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy, stiffness = Spring.StiffnessLow),
    label = "card_scale"
  )

  val elevation by animateDpAsState(if (isPressed) 0.dp else 4.dp * SCALE, label = "elevation")
  val offset by animateDpAsState(if (isPressed) 2.dp else 0.dp, label = "offset")

  Card(
    shape = RectangleShape,
    colors = CardDefaults.cardColors(containerColor = GameBoyColors.MediumGreen),
    modifier = Modifier
      .width(cardWidth)
      .graphicsLayer(scaleX = cardScale, scaleY = cardScale)
      .offset(x = offset, y = offset)
      .combinedClickable(
        interactionSource = interactionSource,
        indication = null,
        onClick = { ON_CLICK(GAME) },
        onLongClick = { ON_LONG_PRESS(GAME) }),
    elevation = CardDefaults.cardElevation(elevation))
  {
    Column(modifier = Modifier.fillMaxWidth().background(GameBoyColors.MediumGreen))
    {
      val url by IMAGE_URL(GAME).collectAsState(initial = null)
      var imageFail by remember { mutableStateOf(false) }

      Box(
        modifier = Modifier
          .fillMaxWidth()
          .aspectRatio(1f)
          .background(if (url.isNullOrEmpty() || imageFail) GameBoyColors.LightGreen else Color.Transparent),
        contentAlignment = Alignment.Center)
      {
        CustomText(TEXT = initials, FONT_SIZE = (36 * SCALE).toInt(), COLOR = GameBoyColors.DarkGreen)

        if (!url.isNullOrEmpty() && !imageFail)
        {
          AsyncImage(
            model = ImageRequest.Builder(App.appModule.context).data(url).crossfade(true).build(),
            contentDescription = GAME.title.trim(),
            contentScale = ContentScale.Crop,
            modifier = Modifier.fillMaxSize(),
            onError = { imageFail = true },
            onSuccess = { imageFail = false })
        }
      }

      Spacer(modifier = Modifier.height(2.dp * SCALE))
      CustomText(TEXT = GAME.title, FONT_SIZE = titleFont, MAX_LINES = 1, MODIFIER = Modifier.padding(horizontal = 4.dp * SCALE, vertical = 0.dp))
      Spacer(modifier = Modifier.height(2.dp * SCALE))
    }
  }
}

@Composable
fun GameList(
  GAMES           : List<Game>,
  MODIFIFER       : Modifier                = Modifier,
  TITLE           : String,
  ON_LONG_PRESS   : (Game) -> Unit          = {},
  ON_CLICK        : (Game) -> Unit          = {},
  GET_URL         : (Game) -> Flow<String?> = { flowOf(null) },
  USE_ROW         : Boolean                 = true,
  SHOW_TITLE      : Boolean                 = true,
  BIG             : Boolean                 = false)
{
  if (GAMES.isNotEmpty())
  {
    if (USE_ROW)
    {
      Column(horizontalAlignment = Alignment.Start, modifier = MODIFIFER)
      {
        if (SHOW_TITLE) CustomText(TITLE)
        LazyRow(horizontalArrangement = Arrangement.spacedBy(8.dp), contentPadding = PaddingValues(horizontal = 16.dp))
        {
          items(GAMES, key = { it.id }) { game ->
            androidx.compose.animation.AnimatedVisibility(
              visible = true,
              enter = fadeIn() + expandHorizontally(),
              modifier = Modifier.animateItem()
            ) {
              GameCard(GAME = game, ON_CLICK = ON_CLICK, ON_LONG_PRESS = ON_LONG_PRESS, IMAGE_URL = GET_URL, BIG = BIG)
            }
          }
        }
      }
    }
    else
    {
      Column(horizontalAlignment = Alignment.Start, modifier = MODIFIFER)
      {
        if (SHOW_TITLE) CustomText(TITLE)

        LazyVerticalGrid(
          columns = GridCells.Adaptive(minSize = if (BIG) 180.dp else 130.dp),
          verticalArrangement = Arrangement.spacedBy(12.dp),
          horizontalArrangement = Arrangement.spacedBy(12.dp),
          contentPadding = PaddingValues(start = 16.dp, end = 16.dp, top = 16.dp, bottom = 100.dp),
          modifier = Modifier.fillMaxSize())
        {
          items(GAMES, key = { it.id }) { game ->
            AnimatedVisibility(
              visible = true,
              enter = fadeIn() + expandVertically(),
              modifier = Modifier.animateItem()
            ) {
              GameCard(GAME = game, ON_CLICK = ON_CLICK, ON_LONG_PRESS = ON_LONG_PRESS, IMAGE_URL = GET_URL, BIG = BIG)
            }
          }
        }
      }
    }
  }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GameActionBottomSheet(
  GAME                      : Game,
  ON_DISMISS                : () -> Unit,
  ON_PLAY                   : () -> Unit,
  ON_FAVORITE               : () -> Unit,
  ON_UPDATE_BOXART          : (String) -> Unit,
  COLLECTIONS               : List<just.somebody.templates.domain.models.GameCollection> = emptyList(),
  ON_ADD_TO_COLLECTION      : (Long) -> Unit = {},
  ON_REMOVE_FROM_COLLECTION : (Long) -> Unit = {},
  IN_COLLECTION_ID          : Long? = null)
{
  // - - - We strip the cache-busting timestamp for the display field to keep it clean for the user.
  var boxArtUrl by remember { mutableStateOf(GAME.boxArtUrl?.substringBefore("?") ?: "") }
  var showCollectionPicker by remember { mutableStateOf(false) }

  ModalBottomSheet(onDismissRequest = ON_DISMISS, containerColor = GameBoyColors.DarkGreen)
  {
    Column(
      modifier = Modifier
        .padding(16.dp)
        .fillMaxWidth()
        .verticalScroll(rememberScrollState()),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.Start)
    {
      CustomText(GAME.title)
      Spacer(modifier = Modifier.height(8.dp))

      if (!showCollectionPicker)
      {
        OutlinedTextField(
          value = boxArtUrl,
          onValueChange = { boxArtUrl = it },
          label = { Text(stringResource(R.string.BOX_ART_URL), fontFamily = MinecraftFontFamily, color = GameBoyColors.DarkGreen) },
          modifier = Modifier.fillMaxWidth().background(color = GameBoyColors.LightGreen),
          textStyle = TextStyle(fontFamily = MinecraftFontFamily, fontSize = 14.sp, color = GameBoyColors.DarkGreen),
          colors = OutlinedTextFieldDefaults.colors(
            focusedBorderColor = GameBoyColors.LightGreen,
            unfocusedBorderColor = GameBoyColors.LightGreen,
            focusedLabelColor = GameBoyColors.Green,
            unfocusedLabelColor = GameBoyColors.Green,
            cursorColor = GameBoyColors.DarkGreen),
          shape = RectangleShape)

        CustomButton(ON_CLICK = { ON_UPDATE_BOXART(boxArtUrl) }, MODIFIER = Modifier.fillMaxWidth())
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.redo), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(stringResource(R.string.UPDATE_BOX_ART), FONT_SIZE = 14, MODIFIER = Modifier) 
          }
        }

        CustomButton(ON_CLICK = ON_PLAY, MODIFIER = Modifier.fillMaxWidth())
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.gamepad), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(stringResource(R.string.PLAY), FONT_SIZE = 14, MODIFIER = Modifier) 
          }
        }

        CustomButton(ON_CLICK = { App.appModule.screenshotManager.openScreenshotsForGame(GAME.title) }, MODIFIER = Modifier.fillMaxWidth())
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.camera), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(stringResource(R.string.VIEW_SCREENSHOTS), FONT_SIZE = 14, MODIFIER = Modifier) 
          }
        }

        if (IN_COLLECTION_ID != null)
        {
          CustomButton(ON_CLICK = { ON_REMOVE_FROM_COLLECTION(IN_COLLECTION_ID) }, MODIFIER = Modifier.fillMaxWidth(), COLOR = GameBoyColors.Error)
          { 
            Row(
              modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
              verticalAlignment = Alignment.CenterVertically,
              horizontalArrangement = Arrangement.Start) {
              Icon(painterResource(R.drawable.trash), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
              Spacer(Modifier.width(12.dp))
              CustomText(stringResource(R.string.REMOVE_FROM_LIST), FONT_SIZE = 14, MODIFIER = Modifier) 
            }
          }
        }

        CustomButton(ON_CLICK = { showCollectionPicker = true }, MODIFIER = Modifier.fillMaxWidth())
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.list), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(stringResource(R.string.ADD_TO_LIST), FONT_SIZE = 14, MODIFIER = Modifier) 
          }
        }

        CustomButton(ON_CLICK = ON_FAVORITE, MODIFIER = Modifier.fillMaxWidth())
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.heart), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(if (!GAME.isFavorite) stringResource(R.string.ADD_FAV) else stringResource(R.string.REMOVE_FAV), FONT_SIZE = 14, MODIFIER = Modifier)
          }
        }

        CustomButton(ON_CLICK = { App.appModule.gameBoy.deleteRamFile(GAME.romUri) }, MODIFIER = Modifier.fillMaxWidth(), COLOR = GameBoyColors.Error)
        { 
          Row(
            modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.Start) {
            Icon(painterResource(R.drawable.trash), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(12.dp))
            CustomText(TEXT = stringResource(R.string.DELTE_SAV), FONT_SIZE = 14, MODIFIER = Modifier)
          }
        }
      }
      else
      {
        Row(verticalAlignment = Alignment.CenterVertically)
        {
          CustomButton(ON_CLICK = { showCollectionPicker = false }, MODIFIER = Modifier.width(60.dp))
          { CustomText("<") }
          Spacer(modifier = Modifier.width(8.dp))
          CustomText(stringResource(R.string.COLLECTIONS))
        }
        Spacer(modifier = Modifier.height(8.dp))
        COLLECTIONS.forEach { collection ->
          CustomButton(ON_CLICK = { ON_ADD_TO_COLLECTION(collection.id); showCollectionPicker = false }, MODIFIER = Modifier.fillMaxWidth())
          { CustomText(collection.name) }
        }
        if (COLLECTIONS.isEmpty())
        {
          CustomText(stringResource(R.string.empty), COLOR = GameBoyColors.Green, MODIFIER = Modifier.padding(16.dp))
        }
      }
    }
  }
}
