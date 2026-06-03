package just.somebody.templates.presentation.screens

import just.somebody.templates.presentation.widgets.GameActionBottomSheet
import just.somebody.templates.presentation.widgets.GameList
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import just.somebody.templates.R
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors

/**
 * Filtered dashboard display view collecting all user-bookmarked game cartridges.
 *
 * Checks continuous state indicators evaluating localized favorites. If the database returns an empty sequence,
 * it renders a standard retro text message. Otherwise, it presents a complete vertical responsive grid structure
 * of the saved games, maintaining full compatibility with the application's global interactive contextual menu sheet.
 *
 * @param VIEW_MODEL State coordinator facilitating persistent database reads and catalog indexing mutations.
 * @param MODIFIFER [Modifier] used to establish dimensional layout bounds or dimension scaling rules.
 */
@Composable
fun FavoriteScreen(
  VIEW_MODEL : GamesViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val favoriteGames  = VIEW_MODEL.favoriteGames.collectAsState()
  val selectedGame   = VIEW_MODEL.selectedGame.collectAsState()
  val collections    = VIEW_MODEL.collections.collectAsState()
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
        FONT_SIZE = 21)
    }
    else
    {
      Column (
        modifier            = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Top,
        horizontalAlignment = Alignment.Start)
      {
        GameList(
          GAMES         = favoriteGames.value,
          TITLE         = stringResource(R.string.FAV),
          ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
          ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game)},
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) },
          USE_ROW       = false,
          SHOW_TITLE    = false)
      }
    }

    selectedGame.value?.let ()
    { game ->
      GameActionBottomSheet(
        GAME             = game,
        ON_DISMISS       = { VIEW_MODEL.selectGame(null)},
        ON_PLAY          =
          {
            VIEW_MODEL.markAsPlayed(game)
            VIEW_MODEL.selectGame(null)
          },
        ON_FAVORITE      =
          {
            VIEW_MODEL.toggleFavorite(game)
            VIEW_MODEL.selectGame(null)
          },
        ON_UPDATE_BOXART = { url -> VIEW_MODEL.updateBoxArtUrl(game, url) },
        COLLECTIONS      = collections.value,
        ON_ADD_TO_COLLECTION = { collectionId ->
          VIEW_MODEL.addToCollection(collectionId, game.id)
          VIEW_MODEL.selectGame(null)
        })
    }
  }
}
