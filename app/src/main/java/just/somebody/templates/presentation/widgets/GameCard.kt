import androidx.compose.foundation.background
import androidx.compose.foundation.combinedClickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
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
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
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

@Composable
fun GameCard(
  GAME          : Game,
  IMAGE_URL     : (Game) -> String?,
  ON_CLICK      : (Game) -> Unit,
  ON_LONG_PRESS : (Game) -> Unit,
  BIG           : Boolean
)
{
  val initials = remember(GAME.title)
  {
    GAME.title
      .split(Regex("\\s+"))
      .take(3)
      .mapNotNull { word -> word.firstOrNull { it.isLetterOrDigit() } }
      .joinToString("") { it.uppercaseChar().toString() }
  }

  Card(
    shape     = RectangleShape,
    modifier  = Modifier
      .width(
        if (BIG) 160.dp
        else     140.dp)
      .combinedClickable(
        onClick     = { ON_CLICK(GAME) } ,
        onLongClick = { ON_LONG_PRESS(GAME) }
      ),
    elevation = CardDefaults.cardElevation(4.dp)
  )
  {
    Column(modifier = Modifier
      .fillMaxWidth()
      .background(GameBoyColors.MediumGreen))
    {
      val url = IMAGE_URL(GAME)
      Box(
        modifier          = Modifier
          .fillMaxWidth()
          .height(
            if (BIG) 600.dp
            else     96.dp)
          .background(
            if (url.isNullOrEmpty()) GameBoyColors.LightGreen
            else                     Color.Transparent),
        contentAlignment = Alignment.Center
      )
      {
        if (!url.isNullOrEmpty())
        {
          AsyncImage(
            model              = ImageRequest.Builder(App.appModule.context)
              .data(IMAGE_URL)
              .crossfade(true)
              .build(),
            contentDescription = GAME.title.trim(),
            contentScale       = ContentScale.Crop,
            modifier           = Modifier.fillMaxSize(),
            error              = painterResource(R.drawable.gameboy)
          )
        }
        else
        {
          CustomText(
            TEXT      = initials,
            FONT_SIZE = 36,
            COLOR     = GameBoyColors.DarkGreen)
        }
      }

      Spacer(modifier = Modifier.height(2.dp))
      CustomText(
        TEXT      = GAME.title,
        FONT_SIZE = 24,
        MAX_LINES = 1,
        MODIFIER  = Modifier.padding(4.dp))
      CustomText(
        TEXT      = GAME.publisher,
        FONT_SIZE = 16,
        MAX_LINES = 1,
        MODIFIER  = Modifier.padding(4.dp))
    }
  }
}

@Composable
fun GameList(
  GAMES           : List<Game>,
  MODIFIFER       : Modifier          = Modifier,
  TITLE           : String,
  ON_LONG_PRESS   : (Game) -> Unit    = {},
  ON_CLICK        : (Game) -> Unit    = {},
  GET_URL         : (Game) -> String? = { null },
  USE_ROW         : Boolean           = true,
  BIG             : Boolean           = false
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
        CustomText(TITLE)
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
        CustomText(TITLE)
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
  ModalBottomSheet(onDismissRequest = ON_DISMISS)
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
