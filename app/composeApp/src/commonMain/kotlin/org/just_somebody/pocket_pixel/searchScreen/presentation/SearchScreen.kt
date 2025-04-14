package org.just_somebody.pocket_pixel.searchScreen.presentation


import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.Game
import org.just_somebody.pocket_pixel.core.GameListUI
import pocketpixel.composeapp.generated.resources.NoInternet
import pocketpixel.composeapp.generated.resources.Res


@Composable
fun SearchScreen(
    MODIFIER   : Modifier = Modifier,
    GO_TO_GAME : (Game) -> Unit,
)
{
    val viewModel : SearchViewModel = viewModel()
    val state by viewModel.state.collectAsStateWithLifecycle()

    Column(
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Top,
        horizontalAlignment = Alignment.CenterHorizontally,
    )
    {
        SearchBar(
            MODIFIER                = Modifier
                .padding(16.dp)
                .fillMaxWidth(),
            SEARCH_QUERY            = state.searchQuery,
            ON_SEARCH_QUERY_CHANGE  = { newQuery -> viewModel.onAction(SearchActions.ChangeSearchTerm(newQuery)) },
            ON_SEARCH_TRIGGER       = { viewModel.onAction(SearchActions.Search) }
        )

        if (state.errorMessage != null || state.searchResults.isEmpty())
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
                GAMES       = state.searchResults,
                ON_CLICK    = { game -> viewModel.onAction(SearchActions.GoToGame(game)); GO_TO_GAME(game) }
            )
        }
    }
}