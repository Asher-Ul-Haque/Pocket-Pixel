package just.somebody.templates.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import just.somebody.templates.R
import just.somebody.templates.domain.models.GameCollection
import just.somebody.templates.presentation.viewModels.CollectionsViewModel
import just.somebody.templates.presentation.widgets.*
import just.somebody.templates.ui.theme.GameBoyColors

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CollectionsScreen(
  VIEW_MODEL : CollectionsViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val collections        by VIEW_MODEL.collections.collectAsState()
  val selectedCollection by VIEW_MODEL.selectedCollection.collectAsState()
  val selectedGame       by VIEW_MODEL.selectedGame.collectAsState()
  var showCreateDialog   by remember { mutableStateOf(false) }

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen))
  {
    Column(
      modifier = Modifier
        .fillMaxSize()
        .padding(16.dp))
    {
      if (selectedCollection == null)
      {
        CustomButton(
          ON_CLICK = { showCreateDialog = true },
          MODIFIER = Modifier.fillMaxWidth().padding(bottom = 16.dp))
        { CustomText(stringResource(R.string.CREATE_COLLECTION)) }

        LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp))
        {
          items(collections)
          { collection ->
            CollectionItem(
              COLLECTION = collection,
              ON_CLICK   = { VIEW_MODEL.selectCollection(collection) },
              ON_DELETE  = { VIEW_MODEL.deleteCollection(collection) })
          }
        }
      }
      else
      {
        Row(
          modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
          verticalAlignment = Alignment.CenterVertically)
        {
          CustomButton(
            ON_CLICK = { VIEW_MODEL.selectCollection(null) },
            MODIFIER = Modifier.width(60.dp))
          { CustomText("<", FONT_SIZE = 18) }
          
          Spacer(modifier = Modifier.width(8.dp))
          CustomText(selectedCollection!!.name, FONT_SIZE = 18)
        }

        GameList(
          GAMES         = selectedCollection!!.games,
          TITLE         = "",
          SHOW_TITLE    = false,
          USE_ROW       = false,
          ON_CLICK      = { VIEW_MODEL.markAsPlayed(it) },
          ON_LONG_PRESS = { VIEW_MODEL.selectGame(it) },
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) })
      }
    }

    if (showCreateDialog)
    {
      CreateCollectionDialog(
        ON_DISMISS = { showCreateDialog = false },
        ON_CREATE  = { name ->
          VIEW_MODEL.createCollection(name)
          showCreateDialog = false
        })
    }

    selectedGame?.let()
    { game ->
      GameActionBottomSheet(
        GAME       = game,
        ON_DISMISS = { VIEW_MODEL.selectGame(null) },
        ON_PLAY    = {
          VIEW_MODEL.markAsPlayed(game)
          VIEW_MODEL.selectGame(null)
        },
        ON_FAVORITE = {
          VIEW_MODEL.selectGame(null)
        },
        ON_UPDATE_BOXART = { /* Implement if needed */ },
        COLLECTIONS = collections,
        ON_ADD_TO_COLLECTION = { collectionId -> 
          VIEW_MODEL.addGameToCollection(collectionId, game.id)
          VIEW_MODEL.selectGame(null)
        },
        ON_REMOVE_FROM_COLLECTION = { collectionId ->
          VIEW_MODEL.removeGameFromCollection(collectionId, game.id)
          VIEW_MODEL.selectGame(null)
        },
        IN_COLLECTION_ID = selectedCollection?.id
      )
    }
  }
}


@Composable
fun CollectionItem(
  COLLECTION : GameCollection,
  ON_CLICK   : () -> Unit,
  ON_DELETE  : () -> Unit)
{
  Row(
    modifier = Modifier
      .fillMaxWidth()
      .background(GameBoyColors.MediumGreen)
      .clickable { ON_CLICK() }
      .padding(12.dp),
    horizontalArrangement = Arrangement.SpaceBetween,
    verticalAlignment     = Alignment.CenterVertically)
  {
    Column()
    {
      CustomText(COLLECTION.name, FONT_SIZE = 16)
      CustomText("${COLLECTION.games.size} Games", FONT_SIZE = 10, COLOR = GameBoyColors.Green)
    }

    if (!COLLECTION.isSystem)
    {
      IconButton(onClick = ON_DELETE)
      {
        Icon(
          painter            = painterResource(R.drawable.trash),
          contentDescription = stringResource(R.string.DELETE_COLLECTION),
          tint                = GameBoyColors.DarkGreen,
          modifier            = Modifier.size(24.dp))
      }
    }
  }
}

@Composable
fun CreateCollectionDialog(
  ON_DISMISS : () -> Unit,
  ON_CREATE  : (String) -> Unit)
{
  var name by remember { mutableStateOf("") }

  AlertDialog(
    onDismissRequest = ON_DISMISS,
    title  = { CustomText(stringResource(R.string.CREATE_COLLECTION)) },
    text   = {
      OutlinedTextField(
        value         = name,
        onValueChange = { name = it },
        placeholder   = { Text(stringResource(R.string.COLLECTION_NAME)) },
        modifier      = Modifier.fillMaxWidth())
    },
    confirmButton = {
      TextButton(onClick = { if (name.isNotBlank()) ON_CREATE(name) })
      { CustomText("OK") }
    },
    dismissButton = {
      TextButton(onClick = ON_DISMISS)
      { CustomText(stringResource(R.string.cancel)) }
    },
    containerColor = GameBoyColors.MediumGreen,
    shape = RectangleShape)
}
