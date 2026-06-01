package just.somebody.templates.presentation.screens

import just.somebody.templates.presentation.widgets.GameActionBottomSheet
import just.somebody.templates.presentation.widgets.GameList
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
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.ui.theme.GameBoyColors
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import android.content.Intent
import android.net.Uri
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.launch
import androidx.core.net.toUri

/**
 * Root library display terminal sorting and showcasing localized collections.
 *
 * This dashboard manages segmented game rosters divided across targeted user categories
 * including bookmarked collections, contextual chronological history files, and newly imported
 * catalogs. When storage registers return an empty state, it displays an unconfigured onboarding view
 * supplying an interactive system [DirectoryPickerLauncher] intent gateway to safely scan external storage paths.
 *
 * @param VIEW_MODEL State coordinator facilitating persistent database reads and catalog indexing mutations.
 * @param MODIFIFER [Modifier] used to establish dimensional layout constraints or boundaries.
 */
@Composable
fun HomeScreen(
  VIEW_MODEL : GamesViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val newGames       = VIEW_MODEL.newGames.collectAsState()
  val favoriteGames  = VIEW_MODEL.favoriteGames.collectAsState()
  val recentGames    = VIEW_MODEL.recentlyPlayedGames.collectAsState()
  val selectedGame   = VIEW_MODEL.selectedGame.collectAsState()
  val settings       = App.appModule.dataStoreManager.settingsFlow.collectAsState(initial = null)
  val context        = LocalContext.current

  val showRatePrompt = remember(settings.value)
  {
    val s = settings.value
    s != null && !s.hasRated && listOf(5, 10, 50, 100).contains(s.launchCount)
  }

  val empty          = newGames.value.isEmpty()      &&
                       favoriteGames.value.isEmpty() &&
                       recentGames.value.isEmpty()
  val storageManager = App.appModule.externalStorageManager
  val scope          = rememberCoroutineScope()
  val pickDirectory  = storageManager.DirectoryPickerLauncher("GAME_BOY_ROMS")
  { uri ->
    if (uri != null) VIEW_MODEL.detectAndInsertRoms()
    else scope.launch()
    {
      SnackbarController.sendEvent(SnackbarEvent(message = context.getString(R.string.no_directory_picked)))
    }
  }

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen),
      contentAlignment = Alignment.Center)
  {
    Column (
      modifier            = Modifier
        .fillMaxSize()
        .verticalScroll(rememberScrollState()),
      horizontalAlignment = Alignment.Start,
      verticalArrangement = Arrangement.Top)
    {
      if (showRatePrompt)
      {
        Card(
          colors    = CardDefaults.cardColors(containerColor = GameBoyColors.MediumGreen),
          shape     = RectangleShape,
          modifier  = Modifier
            .padding(16.dp)
            .fillMaxWidth()
            .border(2.dp, GameBoyColors.Green, RectangleShape))
        {
          Column(
            modifier            = Modifier.padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally)
          {
            CustomText(stringResource(R.string.enjoying_pixel_pocket), FONT_SIZE = 18)
            CustomText(stringResource(R.string.rating_help), FONT_SIZE = 12, COLOR = GameBoyColors.Green)
            Spacer(modifier = Modifier.height(12.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp))
            {
              CustomButton(
                ON_CLICK =
                  {
                    VIEW_MODEL.markAsRated()
                    val intent = Intent(Intent.ACTION_VIEW, "market://details?id=${context.packageName}".toUri())
                    context.startActivity(intent)
                  },
                MODIFIER = Modifier.weight(1f)
                          )
              { CustomText(stringResource(R.string.rate_now), FONT_SIZE = 12) }

              CustomButton(
                ON_CLICK  = { VIEW_MODEL.markAsRated() },
                MODIFIER  = Modifier.weight(1f),
                COLOR     = GameBoyColors.DarkGreen)
              { CustomText(stringResource(R.string.maybe_later), FONT_SIZE = 12) }
            }
          }
        }
      }

      if (empty)
      {
        Column(
          modifier            = Modifier
            .padding(16.dp)
            .border(4.dp, GameBoyColors.Green, RectangleShape)
            .wrapContentHeight(),
          verticalArrangement = Arrangement.Center,
          horizontalAlignment = Alignment.CenterHorizontally)
        {
          CustomText(
            TEXT      = stringResource(R.string.SELECT),
            FONT_SIZE = 21)

          CustomText(stringResource(R.string.SCAN))

          CustomButton( {pickDirectory() })
          { CustomText(stringResource(R.string.DIRECTORY)) }

          Spacer(modifier = Modifier.size(8.dp))
        }
      }
      else
      {
        GameList(
          GAMES         = favoriteGames.value,
          TITLE         = stringResource(R.string.FAV),
          ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
          ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game)},
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) })

        GameList(
          GAMES         = recentGames.value,
          TITLE         = stringResource(R.string.RECENT),
          ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
          ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game) },
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) })

        GameList(
          GAMES         = newGames.value,
          TITLE         = stringResource(R.string.DISCOVER),
          ON_CLICK      = { game -> VIEW_MODEL.markAsPlayed(game) },
          ON_LONG_PRESS = { game -> VIEW_MODEL.selectGame(game) },
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) })
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
        ON_UPDATE_BOXART = { url -> VIEW_MODEL.updateBoxArtUrl(game, url) })
    }
  }
}