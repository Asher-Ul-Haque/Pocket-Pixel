import androidx.compose.foundation.background
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import coil.request.ImageRequest
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.domain.models.Game
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flowOf

@Composable
fun GameCard(
  GAME          : Game,
  IMAGE_URL     : (Game) -> Flow<String?>,
  ON_CLICK      : (Game) -> Unit,
  ON_LONG_PRESS : (Game) -> Unit,
  BIG           : Boolean
)
{
  val SCALE =
    if (BIG) 1f
    else     0.8f

  val initials = remember(GAME.title)
  {
    GAME.title
      .split(Regex("\\s+"))
      .take(3)
      .mapNotNull { word -> word.firstOrNull { it.isLetterOrDigit() } }
      .joinToString("") { it.uppercaseChar().toString() }
  }

  val cardWidth =
    if (BIG)  200.dp * SCALE
    else      140.dp * SCALE
  val titleFont =
    if (BIG)  24
    else      18
  val publisherFont =
    if (BIG)  16
    else      14

  Card(
    shape     = RectangleShape,
    modifier  = Modifier
      .width(cardWidth)
      .combinedClickable(
        onClick     = { ON_CLICK(GAME) },
        onLongClick = { ON_LONG_PRESS(GAME) }
      ),
    elevation = CardDefaults.cardElevation(4.dp * SCALE)
  )
  {
    Column(
      modifier = Modifier
        .fillMaxWidth()
        .background(GameBoyColors.MediumGreen)
    )
    {
      val url by IMAGE_URL(GAME).collectAsState(initial = null)
      var imageFail by remember { mutableStateOf(false) }

      Box(
        modifier          = Modifier
          .fillMaxWidth()
          .aspectRatio(1f)
          .background(
            if (url.isNullOrEmpty() || imageFail) GameBoyColors.LightGreen
            else Color.Transparent
          ),
        contentAlignment = Alignment.Center
      )
      {
        CustomText(
          TEXT      = initials,
          FONT_SIZE = (36 * SCALE).toInt(),
          COLOR     = GameBoyColors.DarkGreen
        )

        if (!url.isNullOrEmpty() && !imageFail)
        {
          AsyncImage(
            model               = ImageRequest.Builder(App.appModule.context)
              .data(url)
              .crossfade(true)
              .build(),
            contentDescription  = GAME.title.trim(),
            contentScale        = ContentScale.Crop,
            modifier            = Modifier.fillMaxSize(),
            onError             = { imageFail = true },
            onSuccess           = { imageFail = false }
          )
        }
      }

      Spacer(modifier = Modifier.height(2.dp * SCALE))

      CustomText(
        TEXT      = GAME.title,
        FONT_SIZE = titleFont,
        MAX_LINES = 1,
        MODIFIER  = Modifier.padding(horizontal = 4.dp * SCALE, vertical = 0.dp)
      )

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
  BIG             : Boolean                 = false
)
{
  if (GAMES.isNotEmpty())
  {
    if (USE_ROW)
    {
      Column (
        horizontalAlignment = Alignment.Start,
        modifier            = MODIFIFER)
      {
        if (SHOW_TITLE) CustomText(TITLE)
        LazyRow(
          horizontalArrangement = Arrangement.spacedBy(8.dp),
          contentPadding        = PaddingValues(horizontal = 16.dp)
        )
        {
          items(GAMES)
          { game ->
            GameCard(
              GAME          = game,
              ON_CLICK      = ON_CLICK,
              ON_LONG_PRESS = ON_LONG_PRESS,
              IMAGE_URL     = GET_URL,
              BIG           = BIG
            )
          }
        }
      }
    }
    else
    {
      Column (
        horizontalAlignment = Alignment.Start,
        modifier            = MODIFIFER)
      {
        if (SHOW_TITLE) CustomText(TITLE)
        LazyVerticalGrid(
          columns               = GridCells.Fixed(2),
          verticalArrangement   = Arrangement.spacedBy(12.dp),
          horizontalArrangement = Arrangement.spacedBy(12.dp),
          contentPadding        = PaddingValues(16.dp),
          modifier              = Modifier.fillMaxHeight()
        )
        {
          items(GAMES)
          { game ->
            GameCard(
              GAME          = game,
              ON_CLICK      = ON_CLICK,
              ON_LONG_PRESS = ON_LONG_PRESS,
              IMAGE_URL     = GET_URL,
              BIG           = BIG)
          }
        }
      }
    }
  }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GameActionBottomSheet(
  GAME        : Game,
  ON_DISMISS  : () -> Unit,
  ON_PLAY     : () -> Unit,
  ON_RESTART  : () -> Unit,
  ON_FAVORITE : () -> Unit,
)
{
  ModalBottomSheet(
    onDismissRequest = ON_DISMISS,
    containerColor   = GameBoyColors.DarkGreen)
  {
    Column(
      modifier            = Modifier.padding(16.dp),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.Start
    )
    {
      CustomText(GAME.title)
      CustomButton(
        ON_CLICK = ON_PLAY,
        MODIFIER = Modifier.fillMaxWidth())
      { CustomText("Play") }

      CustomButton(
        ON_CLICK = ON_RESTART,
        MODIFIER = Modifier.fillMaxWidth())
      { CustomText("Restart") }

      CustomButton(
        ON_CLICK = ON_FAVORITE,
        MODIFIER = Modifier.fillMaxWidth())
      { CustomText(
        if (!GAME.isFavorite) "Favorite"
        else                  "Remove Favorite")
      }
    }
  }
}