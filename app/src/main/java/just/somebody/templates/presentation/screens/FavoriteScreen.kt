package just.somebody.templates.presentation.screens

import GameActionBottomSheet
import GameList
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import kotlinx.coroutines.launch

@Composable
fun FavoriteScreen(
  VIEW_MODEL : GamesViewModel,
  MODIFIFER  : Modifier = Modifier
)
{
  val favoriteGames  = VIEW_MODEL.favoriteGames.collectAsState()
  val selectedGame   = VIEW_MODEL.selectedGame.collectAsState()
  val empty          = favoriteGames.value.isEmpty()

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen),
    contentAlignment = Alignment.Center)
  {
    if (empty)
    {
      CustomText(
        TEXT      = stringResource(R.string.NO_FAV),
        FONT_SIZE = 42)
    }
    else
    {
      Column (
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Top,
        horizontalAlignment = Alignment.Start
      )
      {
        GameList(
          GAMES         = favoriteGames.value,
          TITLE         = stringResource(R.string.FAV),
          ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
          ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game)},
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game.title) },
          USE_ROW       = false,
          SHOW_TITLE    = false
        )
      }
    }

    selectedGame.value?.let ()
    { game ->
      GameActionBottomSheet(
        GAME       = game,
        ON_DISMISS = { VIEW_MODEL.selectGame(null)},
        ON_PLAY    =
        {
          VIEW_MODEL.markAsPlayed(game)
          VIEW_MODEL.selectGame(null)
        },
        ON_FAVORITE  =
        {
          VIEW_MODEL.toggleFavorite(game)
          VIEW_MODEL.selectGame(null)
        },
      )
    }
  }
}