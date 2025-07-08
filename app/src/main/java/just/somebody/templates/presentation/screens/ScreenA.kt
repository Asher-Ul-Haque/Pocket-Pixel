package just.somebody.templates.presentation.screens

import android.Manifest
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.core.app.NotificationCompat
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.getSystemService
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.storage.ExternalStorageManager
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import just.somebody.templates.presentation.viewModels.ScreenAViewModel
import kotlinx.coroutines.launch

@Composable
fun ListGBFilesScreen(storageManager: ExternalStorageManager) {
  var gbFiles by remember { mutableStateOf<List<String>>(emptyList()) }
  var pickedSuccess by remember { mutableStateOf(false) }
  var loading by remember { mutableStateOf(false) }
  val scope = rememberCoroutineScope()

  val pickDirectory = storageManager.DirectoryPickerLauncher("GAME_BOY_ROMS") { uri ->
    if (uri != null) {
      ForgeLogger.info("User picked directory URI: $uri")
      // Trigger re-fetch
      scope.launch {
        val files = storageManager.listFiles("GAME_BOY_ROMS", "gb")
        gbFiles = files.mapNotNull { it.name }
      }
    } else {
      ForgeLogger.warn("Directory picker was cancelled or failed")
    }
  }


  LaunchedEffect(pickedSuccess) {
    if (pickedSuccess) {
      loading = true
      val files = storageManager.listFiles("GAME_BOY_ROMS", "gb")
      gbFiles = files.mapNotNull { it.name }
      if (gbFiles.isEmpty()) {
        ForgeLogger.warn("No .gb files found.")
      } else {
        gbFiles.forEach { ForgeLogger.info("Found .gb file: $it") }
      }
      loading = false
    }
  }

  Column(
    modifier = Modifier
      .fillMaxWidth()
      .padding(16.dp),
    verticalArrangement = Arrangement.spacedBy(8.dp),
    horizontalAlignment = Alignment.CenterHorizontally
  ) {
    Button(onClick = { pickDirectory() }) {
      Text("Pick Directory for .gb Files")
    }

    Spacer(modifier = Modifier.height(16.dp))

    if (loading) {
      Text("Loading files...", style = MaterialTheme.typography.bodyLarge)
    } else if (gbFiles.isEmpty()) {
      Text(
        text = "Nothing found",
        color = Color.Red,
        style = MaterialTheme.typography.bodyLarge
      )
    } else {
      Text("Found ${gbFiles.size} .gb files:")
      LazyColumn(
        modifier = Modifier
          .fillMaxWidth()
          .heightIn(min = 100.dp, max = 400.dp) // Ensures it doesn't collapse
      ) {
        items(gbFiles) { fileName ->
          Text(
            text = fileName,
            modifier = Modifier.padding(8.dp)
          )
        }
      }
    }
  }
}

@Composable
fun ScreenA(
  MODIFIER: Modifier = Modifier,
  VIEW_MODEL: ScreenAViewModel
) {
  val scope = rememberCoroutineScope()
  val storageManager = App.appModule.externalStorageManager
  val askPermission = remember { mutableStateOf(false) }
  val isConnected by VIEW_MODEL.isConnected.collectAsState()

  Column(
    modifier = MODIFIER
      .fillMaxSize()
      .padding(16.dp),
    horizontalAlignment = Alignment.CenterHorizontally,
    verticalArrangement = Arrangement.spacedBy(16.dp)
  ) {
    // Show Snackbar Button
    Button(onClick = {
      scope.launch {
        SnackbarController.sendEvent(
          EVENT = SnackbarEvent(message = "Hello!, world")
        )
      }
    }) {
      Text("Show Snackbar")
    }

    // Snackbar with action
    Button(onClick = { VIEW_MODEL.goAway() }) {
      Text("Show Snackbar with Action")
    }

    // File Picker
    val pickFile = storageManager.FilePickerLauncher(
      MIME_TYPES = arrayOf(ExternalStorageManager.MIME_IMAGE)
    ) { uri ->
      if (uri != null) {
        ForgeLogger.info("Picked file: $uri")
      } else {
        ForgeLogger.warn("User cancelled file picker")
      }
    }

    Button(onClick = { pickFile() }) {
      Text("Pick Image File")
    }

    // Directory Picker & .gb file list
    ListGBFilesScreen(storageManager)

    // Ask Camera Permission
    Button(onClick = {
      askPermission.value = true
      ForgeLogger.trace("Playing click sound")
      scope.launch { SoundController.playSound(SoundEffect.Click) }
    }) {
      Text("Ask Notification Permission")
    }

    if (Build.VERSION.SDK_INT >= 33)
    {
      App.appModule.permissionManager.RequestPermissionIfNeeded(
        PERMISSION = android.Manifest.permission.POST_NOTIFICATIONS,
        ON_GRANT = { /* Handle permission granted */ },
        TRIGGER = askPermission.value,
        ON_TRIGGER = { askPermission.value = !askPermission.value },
        GO_TO_SETTINGS = true
      )
    }

    Button(onClick =
    {
      if (Build.VERSION.SDK_INT >= 33 && !App.appModule.permissionManager.hasPermission(App.appModule.context, Manifest.permission.POST_NOTIFICATIONS))
      { return@Button }
      val notificationManager = App.appModule.context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
      val notification = NotificationCompat.Builder(App.appModule.context, "channel_ID")
        .setContentText("This is some content text")
        .setContentTitle("Hello WOrld!")
        .setSmallIcon(R.drawable.logo)
        .build()
      notificationManager.notify(1, notification)
    })
    { Text("Show notification") }

    // Connectivity status
    Text("Is Connected: $isConnected")
  }
}
