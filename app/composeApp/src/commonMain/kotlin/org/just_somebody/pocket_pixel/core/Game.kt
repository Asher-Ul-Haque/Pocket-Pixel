package org.just_somebody.pocket_pixel.core

import androidx.compose.foundation.Image
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.LazyListState
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.KeyboardArrowRight
import androidx.compose.material3.SegmentedButtonDefaults.Icon
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.BlendMode
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.vector.rememberVectorPainter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil3.compose.rememberAsyncImagePainter
import kotlinx.serialization.Serializable
import org.jetbrains.compose.resources.DrawableResource
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import pocketpixel.composeapp.generated.resources.NoInternet
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.heart
import pocketpixel.composeapp.generated.resources.trophy
import kotlin.Result

@Serializable
data class Game
(
  val releaseYear : Int     = 0,
  val title       : String  = "",
  val publisher   : String  = "",
  val description : String  = "",
  val imageUrl    : String  = ""
);

@Composable
fun GameUI(
  GAME      : Game,
  ON_CLICK  : () -> Unit,
  MODIFIER  : Modifier = Modifier
)
{
  Surface(
    shape     = RectangleShape,
    modifier  = MODIFIER.clickable(onClick = ON_CLICK),
    color     = GameBoyColors.MediumGreen
  )
  {
    Row(
      modifier              = Modifier
        .padding(16.dp)
        .fillMaxWidth()
        .height(IntrinsicSize.Min),
      verticalAlignment     = Alignment.CenterVertically,
      horizontalArrangement = Arrangement.spacedBy(16.dp)
    )
    {
      // - - - the box art
      Box(
        modifier          = Modifier.height(100.dp),
        contentAlignment  = Alignment.Center
      )
      {
        var imageLoadResult by remember { mutableStateOf<Result<Painter>?>(null) }
        val painter = rememberAsyncImagePainter(
          model       = GAME.imageUrl,
          onSuccess   = { imageLoadResult = Result.success(it.painter) },
          onError     = { imageLoadResult = Result.failure(it.result.throwable) }
        )

        if (imageLoadResult != null)
        {
          Image(
            painter             =
              if (imageLoadResult!!.isSuccess) painter
              else                             { painterResource(Res.drawable.NoInternet) },
            contentDescription  = GAME.title,
            contentScale        =
              if (imageLoadResult!!.isSuccess)  ContentScale.Crop
              else                              ContentScale.Fit,
            modifier            = Modifier
              .aspectRatio(1f)
              .height(64.dp)
              .border(4.dp, Color.Black)
              .graphicsLayer { this.alpha = 0.8f } ,
            colorFilter = (ColorFilter.tint(GameBoyColors.Green, BlendMode.Multiply))
          )
        }
      }

      // - - - title
      Column(
        verticalArrangement = Arrangement.Center,
        modifier            = Modifier
          .fillMaxHeight()
          .weight(1f),
      )
      {
        Text(
          text          = GAME.title,
          color         = GameBoyColors.LightGreen,
          fontSize      = 32.sp,
          fontFamily    = PokeFontFamily(),
          maxLines      = 2,
          overflow      = TextOverflow.Ellipsis
        )

        Text(
          text          = GAME.publisher,
          color         = GameBoyColors.LightGreen,
          fontSize      = 24.sp,
          fontFamily    = PokeFontFamily(),
          maxLines      = 2,
          overflow      = TextOverflow.Ellipsis
        )

        Text(
          text          = GAME.description,
          color         = GameBoyColors.LightGreen,
          fontSize      = 24.sp,
          fontFamily    = PokeFontFamily(),
          maxLines      = 2,
          overflow      = TextOverflow.Ellipsis
        )
      }

      // - - - the button
      androidx.compose.material3.Icon(
        painter             = painterResource(Res.drawable.trophy),
        contentDescription  = "Play",
        tint                = GameBoyColors.LightGreen,
        modifier            = Modifier.size(24.dp)
      )
    }
  }
}

@Composable
fun GameListUI(
  GAMES          : List<Game>,
  ON_CLICK      : (Game) -> Unit,
  MODIFIER      : Modifier = Modifier,
  SCROLL_STATE  : LazyListState = rememberLazyListState()
)
{
  LazyColumn (
    modifier            = MODIFIER,
    state               = SCROLL_STATE,
    verticalArrangement = Arrangement.spacedBy(12.dp),
    horizontalAlignment = Alignment.CenterHorizontally
  )
  {
    items(
      items = GAMES,
      key = { it.title }
    )
    { game ->
      GameUI(
        GAME      = game,
        ON_CLICK  = { ON_CLICK(game) },
        MODIFIER  = Modifier
          .widthIn(max = 700.dp)
          .fillMaxWidth()
          .padding(horizontal = 16.dp)
      )
    }
  }
}