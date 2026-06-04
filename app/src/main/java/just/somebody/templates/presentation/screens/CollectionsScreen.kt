package just.somebody.templates.presentation.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import just.somebody.templates.presentation.viewModels.CollectionsViewModel
import just.somebody.templates.presentation.widgets.*
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily

@Composable
fun CollectionsScreen(
  VIEW_MODEL : CollectionsViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val collections  by VIEW_MODEL.collections.collectAsState()
  val selectedGame by VIEW_MODEL.selectedGame.collectAsState()
  var showCreateDialog by remember { mutableStateOf(false) }
  val scope = rememberCoroutineScope()

  LaunchedEffect(selectedGame) {
    if (selectedGame != null) {
      SoundController.playSound(SoundEffect.Menu)
    }
  }

  Box(modifier = MODIFIFER
    .fillMaxSize()
    .background(GameBoyColors.DarkGreen))
  {
    Column(
      modifier = Modifier
        .fillMaxSize()
        .verticalScroll(rememberScrollState()))
    {
      CustomButton(
        ON_CLICK = { showCreateDialog = true },
        MODIFIER = Modifier.fillMaxWidth().padding(16.dp))
      { 
        Row(verticalAlignment = Alignment.CenterVertically) {
          Icon(painterResource(R.drawable.list), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(20.dp))
          Spacer(Modifier.width(8.dp))
          CustomText(stringResource(R.string.CREATE_COLLECTION), FONT_SIZE = 14) 
        }
      }

      collections.filter { !it.isSystem }.forEach { collection ->
        Row(
          modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp),
          verticalAlignment = Alignment.CenterVertically,
          horizontalArrangement = Arrangement.SpaceBetween)
        {
          CustomText(collection.name, FONT_SIZE = 18)
          
          IconButton(onClick = { VIEW_MODEL.deleteCollection(collection) })
          {
            Icon(
              painter            = painterResource(R.drawable.trash),
              contentDescription = stringResource(R.string.DELETE_COLLECTION),
              tint                = GameBoyColors.Green,
              modifier            = Modifier.size(32.dp))
          }
        }

        GameList(
          GAMES         = collection.games,
          TITLE         = "",
          SHOW_TITLE    = false,
          USE_ROW       = true,
          BIG           = true,
          ON_CLICK      = { VIEW_MODEL.markAsPlayed(it) },
          ON_LONG_PRESS = { VIEW_MODEL.selectGame(it) },
          GET_URL       = { game -> VIEW_MODEL.getBoxArtFlow(game) })
        
        Spacer(modifier = Modifier.height(16.dp))
      }
      
      Spacer(modifier = Modifier.height(80.dp))
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
      // Find which collection this game belongs to (for removal)
      val inCollection = collections.find { coll -> coll.games.any { it.id == game.id } }
      
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
        COLLECTIONS = collections.filter { !it.isSystem },
        ON_ADD_TO_COLLECTION = { collectionId -> 
          VIEW_MODEL.addGameToCollection(collectionId, game.id)
          VIEW_MODEL.selectGame(null)
        },
        ON_REMOVE_FROM_COLLECTION = { collectionId ->
          VIEW_MODEL.removeGameFromCollection(collectionId, game.id)
          VIEW_MODEL.selectGame(null)
        },
        IN_COLLECTION_ID = inCollection?.id
      )
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
        placeholder   = { Text(stringResource(R.string.COLLECTION_NAME), fontFamily = MinecraftFontFamily, color = GameBoyColors.DarkGreen) },
        modifier      = Modifier.fillMaxWidth().background(color = GameBoyColors.LightGreen),
        textStyle     = TextStyle(fontFamily = MinecraftFontFamily, fontSize = 14.sp, color = GameBoyColors.DarkGreen),
        colors        = OutlinedTextFieldDefaults.colors(
          focusedTextColor     = GameBoyColors.DarkGreen,
          unfocusedTextColor   = GameBoyColors.DarkGreen,
          focusedBorderColor   = GameBoyColors.LightGreen,
          unfocusedBorderColor = GameBoyColors.LightGreen,
          focusedLabelColor    = GameBoyColors.Green,
          unfocusedLabelColor  = GameBoyColors.Green,
          cursorColor          = GameBoyColors.DarkGreen),
        shape         = RectangleShape)
    },
    confirmButton = {
      CustomButton(
        ON_CLICK = { if (name.isNotBlank()) ON_CREATE(name) },
        MODIFIER = Modifier.width(80.dp))
      { CustomText("OK") }
    },
    dismissButton = {
      CustomButton(
        ON_CLICK = ON_DISMISS,
        MODIFIER = Modifier.width(100.dp),
        COLOR    = GameBoyColors.DarkGreen)
      { CustomText(stringResource(R.string.cancel)) }
    },
    containerColor = GameBoyColors.MediumGreen,
    shape = RectangleShape)
}
