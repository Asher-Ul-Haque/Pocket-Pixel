package org.just_somebody.pocket_pixel.favoritesScreen.presentation


import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.core.GameListUI
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import org.just_somebody.pocket_pixel.depInj.getGamer
import org.just_somebody.pocket_pixel.searchScreen.presentation.SearchActions
import org.just_somebody.pocket_pixel.searchScreen.presentation.SearchBar
import pocketpixel.composeapp.generated.resources.NoInternet
import pocketpixel.composeapp.generated.resources.Res


@Composable
fun FavoritesScreen(
    MODIFIER    : Modifier = Modifier,
    GO_TO_GAME  : (Game) -> Unit)
{
    val viewModel : FavoritesViewModel = viewModel()
    val state by viewModel.state.collectAsStateWithLifecycle()

    LaunchedEffect(true) { viewModel.onAction(FavoriteActions.GetFavorites(getGamer())) }

    Column(
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Top,
        horizontalAlignment = Alignment.CenterHorizontally,
    )
    {
        Text(
            modifier    = Modifier.padding(16.dp),
            text        = "Favorites",
            color       = GameBoyColors.LightGreen,
            fontSize    = 32.sp,
            fontStyle   = FontStyle.Italic,
            fontFamily  = PokeFontFamily(),
        )

        SearchBar(
            MODIFIER                = Modifier
                .padding(16.dp)
                .fillMaxWidth(),
            SEARCH_QUERY            = state.searchQuery,
            ON_SEARCH_QUERY_CHANGE  = { newQuery -> viewModel.onAction(FavoriteActions.ChangeSearchTerm(newQuery)) },
            ON_SEARCH_TRIGGER       = { viewModel.onAction(FavoriteActions.Filter) }
        )

        if (state.errorMessage != null || state.filteredResults.isEmpty())
        {
            Image(
                painter             = painterResource(Res.drawable.NoInternet),
                contentDescription  = null,
                modifier            = Modifier.size(256.dp)
            )
        }
        else
        {
            GameListUI(
                GAMES       = state.filteredResults,
                ON_CLICK    = { game -> viewModel.onAction(FavoriteActions.GoToGame(game)); GO_TO_GAME(game) }
            )
        }
    }
}