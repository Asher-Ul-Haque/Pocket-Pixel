package org.just_somebody.pocket_pixel.cartridgeScreen.presentation
import CustomButton
import androidx.compose.foundation.Image
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.BlendMode
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil3.compose.rememberAsyncImagePainter
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import org.just_somebody.pocket_pixel.depInj.getGame
import pocketpixel.composeapp.generated.resources.NoImage
import pocketpixel.composeapp.generated.resources.NoInternet
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.cartridge

@Composable
fun CartridgeUI(
    MODIFIER    : Modifier = Modifier,
    START_GAME  : () -> Unit)
{
    val game = getGame()

    BoxWithConstraints(
        modifier = MODIFIER
            .fillMaxSize()
            .shadow(
                elevation       = 8.dp,
                ambientColor    = GameBoyColors.DarkGreen,
                spotColor       = GameBoyColors.DarkGreen
            )
    )
    {
        val boxHeight   = maxHeight

        // - - - Background image
        Image(
            painter             = painterResource(Res.drawable.cartridge),
            contentDescription  = null,
            modifier            = Modifier.fillMaxSize(),
            contentScale        = ContentScale.Fit
        )

        // - - - Foreground content
        Column(
            modifier            = Modifier
                .fillMaxSize()
                .padding(top     = boxHeight * 0.2f),
            horizontalAlignment = Alignment.CenterHorizontally
        )
        {
            Text(
                text        = game.title.ifEmpty { "No Title" },
                color       = GameBoyColors.DarkGreen,
                fontSize    = 32.sp,
                fontFamily  = PokeFontFamily(),
                maxLines    = 1,
                overflow    = TextOverflow.Ellipsis,
                textAlign   = TextAlign.Center,
                fontWeight  = FontWeight.ExtraBold,
                modifier    = Modifier
                    .fillMaxWidth(0.7f)
                    .offset(y = -(5).dp)
            )

            Text(
                text        = game.publisher.ifEmpty { "No Publisher" },
                color       = GameBoyColors.MediumGreen,
                fontSize    = 24.sp,
                fontFamily  = PokeFontFamily(),
                maxLines    = 1,
                overflow    = TextOverflow.Ellipsis,
                textAlign   = TextAlign.Center,
                modifier    = Modifier
                    .fillMaxWidth(0.6f)
                    .offset(y = -(5).dp)
            )

            Spacer(modifier = Modifier.height(boxHeight * 0.05f))

            var imageLoadResult by remember { mutableStateOf<Result<Painter>?>(null) }
            val painter = rememberAsyncImagePainter(
                model       = game.imageUrl,
                onSuccess   = { imageLoadResult = Result.success(it.painter) },
                onError     = { imageLoadResult = Result.failure(it.result.throwable) }
            )

            if (imageLoadResult != null)
            {
                Image(
                    painter             =
                        if (imageLoadResult!!.isSuccess) painter
                        else                             { painterResource(Res.drawable.NoImage) },
                    contentDescription  = game.title,
                    contentScale        = ContentScale.Fit,
                    modifier            = Modifier
                        .width(210.dp)
                        .height(210.dp)
                        .offset(
                            x = (-2).dp,
                            y = (12.dp)),
                    colorFilter         =
                        if (imageLoadResult!!.isSuccess) (ColorFilter.tint(GameBoyColors.Green, BlendMode.Multiply))
                        else                             null
                )
            }

            Spacer(modifier = Modifier.height(boxHeight * 0.08f))

            Text(
                text        = game.description.ifEmpty { "No Description" },
                color       = GameBoyColors.LightGreen,
                textAlign   = TextAlign.Center,
                fontSize    = 20.sp,
                fontFamily  = PokeFontFamily(),
                maxLines    = 3,
                overflow    = TextOverflow.Ellipsis,
                modifier    = Modifier.fillMaxWidth(0.7f) )

            Spacer(modifier = Modifier.height(boxHeight * 0.16f))

            CustomButton(
                ON_CLICK    = START_GAME,
                CONTENT     =
                    {
                        Text(
                            modifier    = Modifier.padding(16.dp),
                            text        = "PLAY",
                            textAlign   = TextAlign.Center,
                            color       = GameBoyColors.LightGreen,
                            fontSize    = 24.sp,
                            fontFamily  = PokeFontFamily()
                        )
                    },
                MODIFIER    = Modifier
                    .width(256.dp)
            )
        }
    }
}
