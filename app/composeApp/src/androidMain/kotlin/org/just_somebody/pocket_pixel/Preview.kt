package org.just_somebody.pocket_pixel

import androidx.compose.runtime.Composable
import androidx.compose.ui.tooling.preview.Preview
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.exploreScreen.presentation.CartridgeUI

@Preview
@Composable
private fun GameUIPrev()
{
    val game = Game(
        description = "Nice",
        releaseYear = 12,
        title = "Hoga",
        publisher = "Publisher",
        imageUrl = "https://m.media-amazon.com/images/M/MV5BZjBiZThkOTMtYWU5My00ZDg2LWI1NzktN2Q3MDMwODUwOTZkXkEyXkFqcGc@._V1_FMjpg_UX1000_.jpg"
    )
    
    CartridgeUI(GAME = game)
}