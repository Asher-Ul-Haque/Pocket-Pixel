package org.just_somebody.pocket_pixel.cartridgeScreen.presentation

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.favoritesScreen.presentation.FavoriteActions
import org.just_somebody.pocket_pixel.searchScreen.presentation.SearchBar
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.cartridge

@Composable
fun CartridgeUI(GAME : Game, MODIFIER : Modifier)
{
    Box()
    {
        Image(
            painter = painterResource(Res.drawable.cartridge),
            contentDescription = null
        )
        Column()
        {
            Text(text = "Test thingy")
        }
    }
}