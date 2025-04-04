package org.just_somebody.pocket_pixel.exploreScreen.presentation

import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.IntrinsicSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil3.compose.rememberAsyncImagePainter
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import org.just_somebody.pocket_pixel.exploreScreen.domain.Game
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.cartridge
import pocketpixel.composeapp.generated.resources.ogreGirl

@Composable
fun CartridgeUI(MODIFIER : Modifier = Modifier, GAME : Game)
{
    Box(
        modifier            = Modifier.height(IntrinsicSize.Min),
        contentAlignment    = Alignment.Center
    )
    {
        Image(
            painter             = painterResource(Res.drawable.cartridge),
            contentDescription  = null,
            modifier            = Modifier.size(256.dp)
        )

        Column ()
        {
            Text(
                text        = GAME.title,
                color       = Color.White,
                fontSize    = 32.sp,
                fontFamily  = PokeFontFamily(),
                fontStyle   = FontStyle.Italic
            )

            Box(
                modifier            = Modifier.height(100.dp),
                contentAlignment    = Alignment.Center
            )
            {
                var imageLoadResult by remember ()
                { mutableStateOf<Result<Painter>?>(null) }

                val painter = rememberAsyncImagePainter(
                    model       = GAME.imageUrl,
                    onSuccess   = { imageLoadResult = Result.success(it.painter) },
                    onError     = { imageLoadResult = Result.failure(it.result.throwable) }
                )

                when (val result = imageLoadResult)
                {
                    null -> CircularProgressIndicator()
                    else ->
                    {
                        Image(
                            painter             =   if (result.isSuccess)   painter
                                                    else                    painterResource(Res.drawable.ogreGirl),
                            contentDescription  = null
                        )
                    }
                }
            }

            Image(
                painter             = painterResource(Res.drawable.cartridge),
                contentDescription  = null,
                modifier            = Modifier.size(256.dp)
            )

            Text(
                text        = GAME.description,
                color       = Color.White,
                fontSize    = 32.sp,
                fontFamily  = PokeFontFamily(),
                fontStyle   = FontStyle.Italic
            )

            Row ()
            {
                Button(onClick = {}){}
                Button(onClick = {}){}
            }
        }
    }
}