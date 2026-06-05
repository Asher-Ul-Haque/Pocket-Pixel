package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.domain.models.Game
import just.somebody.templates.domain.models.GameCollection
import just.somebody.templates.domain.repositories.CollectionRepository
import just.somebody.templates.domain.repositories.GameRepository
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.flowOf
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.launch

/**
 * State coordinator managing the creation, deletion, and organization of custom game collections.
 */
class CollectionsViewModel(
  private val collectionRepo : CollectionRepository,
  private val gameRepo       : GameRepository
) : ViewModel()
{
  val collections : StateFlow<List<GameCollection>> = 
    collectionRepo.getAllCollections().let { flow ->
      val state = MutableStateFlow<List<GameCollection>>(emptyList())
      viewModelScope.launch { flow.collect { state.value = it } }
      state
    }

  private val _selectedCollection = MutableStateFlow<GameCollection?>(null)
  val selectedCollection: StateFlow<GameCollection?> = _selectedCollection

  private val _selectedGame = MutableStateFlow<Game?>(null)
  val selectedGame: StateFlow<Game?> = _selectedGame

  fun selectCollection(collection: GameCollection?)
  {
    _selectedCollection.value = collection
  }

  fun selectGame(game: Game?)
  {
    _selectedGame.value = game
  }

  fun createCollection(name: String)
  {
    viewModelScope.launch {
      collectionRepo.createCollection(name)
    }
  }

  fun deleteCollection(collection: GameCollection)
  {
    viewModelScope.launch {
      collectionRepo.deleteCollection(collection)
      if (_selectedCollection.value?.id == collection.id) {
        _selectedCollection.value = null
      }
    }
  }

  fun addGameToCollection(collectionId: Long, gameId: Long)
  {
    viewModelScope.launch {
      collectionRepo.addGameToCollection(collectionId, gameId)
      SoundController.playSound(SoundEffect.Ping2)
      SnackbarController.sendEvent(SnackbarEvent("Added to collection"))
    }
  }

  fun removeGameFromCollection(collectionId: Long, gameId: Long)
  {
    viewModelScope.launch {
      collectionRepo.removeGameFromCollection(collectionId, gameId)
      SoundController.playSound(SoundEffect.Ping2)
      SnackbarController.sendEvent(SnackbarEvent("Removed from collection"))
      // Refresh current collection if selected
      _selectedCollection.value?.let { current ->
        if (current.id == collectionId) {
          // The flow should handle this if it's observing the DB
        }
      }
    }
  }

  fun getBoxArtFlow(game: Game): Flow<String?>
  {
    if (game.boxArtUrl != null) return flowOf(game.boxArtUrl)
    return App.appModule.boxArtFetcher.fetchBoxArt(game.title).map { url ->
      if (url != null && game.boxArtUrl == null) {
        viewModelScope.launch {
          gameRepo.updateGame(game.copy(boxArtUrl = url))
        }
      }
      url
    }
  }

  fun markAsPlayed(game: Game)
  {
    viewModelScope.launch {
      App.appModule.navigator.navigate(just.somebody.templates.presentation.screens.Destination.Emulator(game.romUri))
    }
  }
}
