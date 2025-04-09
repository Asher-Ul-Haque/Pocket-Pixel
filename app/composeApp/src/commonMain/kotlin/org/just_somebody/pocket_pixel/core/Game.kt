package org.just_somebody.pocket_pixel.core

import androidx.compose.foundation.Image
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
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil3.compose.rememberAsyncImagePainter
import kotlinx.serialization.Serializable
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import pocketpixel.composeapp.generated.resources.NoInternet
import pocketpixel.composeapp.generated.resources.Res
import kotlin.Result

@Serializable
data class Game
(
  val releaseYear : Int,
  val title       : String,
  val publisher   : String,
  val description : String,
  val imageUrl    : String
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
    color     = GameBoyColors.Green
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
        modifier = Modifier
          .height(100.dp),
        contentAlignment = Alignment.Center
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
            modifier = Modifier
              .aspectRatio(
                ratio                       = 0.65f,
                matchHeightConstraintsFirst = true
              )
          )
        }
      }

      // - - - title
      Column(
        modifier            = Modifier
          .fillMaxHeight()
          .weight(1f),
        verticalArrangement = Arrangement.Center
      )
      {
        Text(
          text          = GAME.title,
          color         = GameBoyColors.LightGreen,
          fontSize      = 72.sp,
          fontFamily    = PokeFontFamily(),
          maxLines      = 2,
          overflow      = TextOverflow.Ellipsis
        )
      }
    }
  }
}