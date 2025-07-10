package just.somebody.templates.presentation.widgets

/*
val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
val scope = rememberCoroutineScope()
var selectedGame by remember { mutableStateOf<Game?>(null) }

selectedGame?.let { game ->
  ModalBottomSheet(
    onDismissRequest = { selectedGame = null },
    sheetState = sheetState
  ) {
    Column(
      modifier = Modifier
        .fillMaxWidth()
        .padding(16.dp)
    ) {
      Text("Options for ${game.title}", style = MaterialTheme.typography.titleMedium)
      Spacer(Modifier.height(8.dp))

      Button(onClick = {
        /* Resume logic */
        selectedGame = null
      }) { Text("Resume") }

      Button(onClick = {
        /* Restart logic */
        selectedGame = null
      }) { Text("Restart") }

      Button(onClick = {
        /* Toggle favorite */
        VIEW_MODEL.toggleFavorite(game)
        selectedGame = null
      }) { Text(if (game.isFavorite) "Remove Favorite" else "Add to Favorites") }
    }
  }
}
*/