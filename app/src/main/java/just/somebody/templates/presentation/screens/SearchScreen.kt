package just.somebody.templates.presentation.screens

import just.somebody.templates.presentation.widgets.GameActionBottomSheet
import just.somebody.templates.presentation.widgets.GameList
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.R
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.SearchBar
import just.somebody.templates.ui.theme.GameBoyColors

/**
 * Interactive search interface for querying the local game database library.
 *
 * Exposes a structured input bar linked to the backing query pipeline. If no database record
 * matches the current character array, it displays a standard localized feedback message.
 * Otherwise, it lists the matching results inside a multi-column scroll layout, supporting
 * secondary long-press actions to open a contextual utility sheet.
 *
 * @param VIEW_MODEL State coordinator managing query evaluation and matching catalog records.
 * @param MODIFIFER [Modifier] used to establish positional layout bounds or dimension scaling rules.
 */
@Composable
fun SearchScreen(
  VIEW_MODEL : GamesViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val searchQuery  = VIEW_MODEL.searchQuery.collectAsState()
  val results      = VIEW_MODEL.searchResults.collectAsState()
  val selectedGame = VIEW_MODEL.selectedGame.collectAsState()
  val collections  = VIEW_MODEL.collections.collectAsState()
  val empty        = results.value.isEmpty()

  Box(
    modifier = MODIFIFER
      .fillMaxSize()
      .background(GameBoyColors.DarkGreen),
    contentAlignment = Alignment.Center
     )
  {
    Column(
      modifier            = Modifier.fillMaxSize(),
      verticalArrangement = Arrangement.Top,
      horizontalAlignment = Alignment.Start
          )
    {
      SearchBar(
        SEARCH_QUERY           = searchQuery.value,
        ON_SEARCH_QUERY_CHANGE = { VIEW_MODEL.updateSearchQuery(it) },
        ON_SEARCH_TRIGGER      = {},
        MODIFIER               = Modifier
          .padding(16.dp)
          .fillMaxWidth())

      if (empty)
      {
        Box(
          modifier         = Modifier.fillMaxSize(),
          contentAlignment = Alignment.TopCenter)
        {
          CustomText(
            TEXT      = stringResource(R.string.NO_RES),
            FONT_SIZE = 16)
        }
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
            GAMES         = results.value,
            TITLE         = searchQuery.value,
            ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
            ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game) },
            USE_ROW       = false,
            GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) },
            SHOW_TITLE    = false)
        }
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
