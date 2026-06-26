package just.somebody.templates.presentation.screens

import android.app.Activity
import android.content.Intent
import android.net.Uri
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.BasicAlertDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.navigation.NavigationAction
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.viewModels.AchievementViewModel
import just.somebody.templates.presentation.viewModels.BrowseViewModel
import just.somebody.templates.presentation.viewModels.CollectionsViewModel
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.viewModels.viewModelFactory
import just.somebody.templates.presentation.widgets.CustomButton
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.NavBar
import just.somebody.templates.ui.theme.GameBoyColors

@Composable
private fun FAQItemView(QUESTION: String, ANSWER: String, IS_EXPANDED: Boolean, ON_TOGGLE: () -> Unit) {
  Column(modifier = Modifier.fillMaxWidth()) {
    CustomButton(
      ON_CLICK = ON_TOGGLE,
      MODIFIER = Modifier.fillMaxWidth(),
      COLOR = if (IS_EXPANDED) GameBoyColors.MediumGreen else GameBoyColors.DarkGreen
    ) {
      CustomText(QUESTION, FONT_SIZE = 14, MODIFIER = Modifier.fillMaxWidth().padding(8.dp))
    }
    if (IS_EXPANDED) {
      Box(
        modifier = Modifier
          .fillMaxWidth()
          .padding(top = 4.dp)
          .background(GameBoyColors.MediumGreen.copy(alpha = 0.3f))
          .border(1.dp, GameBoyColors.MediumGreen, RectangleShape)
          .padding(12.dp)
      ) {
        CustomText(ANSWER, FONT_SIZE = 12, COLOR = GameBoyColors.LightGreen, MODIFIER = Modifier)
      }
    }
  }
}

/**
 * Root navigation host and screen layout container for the application library dashboard.
 *
 * This composable initializes all core execution architecture layers ([GamesViewModel], [SettingsViewModel],
 * [EmulatorViewModel]). It tracks window state flows to dynamically flip system status bars between full
 * immersion (during game emulation) and structured shell scaffolding. Additionally, it intercepts cross-layer
 * navigation intents and parses orientation variables to present a fluid dashboard configuration.
 *
 * @param SNACK Floating message controller state link attached to the [Scaffold] layer.
 * @param VIEW_MODEL State supervisor tracking menu indexes and active display modes.
 * @param MODIFIFER [Modifier] used to establish positional layout logic or boundary dimensions.
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BrowseScreen(
  SNACK      : SnackbarHostState,
  VIEW_MODEL : BrowseViewModel,
  MODIFIFER  : Modifier = Modifier)
{
  val state by VIEW_MODEL.browseState.collectAsState()

  val context = LocalContext.current
  LaunchedEffect(state.showBars)
  {
    val window      = (context as? Activity)?.window ?: return@LaunchedEffect
    val controller  = WindowCompat.getInsetsController(window, window.decorView)
    if (state.showBars)
    {
      controller.show(WindowInsetsCompat.Type.systemBars())
    }
    else
    {
      controller.hide(WindowInsetsCompat.Type.systemBars())
      controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
    }
  }

  val gamesViewModel =
    viewModel<GamesViewModel>(factory = viewModelFactory()
    { GamesViewModel(App.appModule.repo)
    })
  val collectionsViewModel =
    viewModel<CollectionsViewModel>(factory = viewModelFactory()
    {
      CollectionsViewModel(
        collectionRepo = App.appModule.collectionRepo,
        gameRepo       = App.appModule.repo)
    })
  val settingsViewModel =
    viewModel<SettingsViewModel>(
      factory = viewModelFactory()
      {
        SettingsViewModel(
          REPO            = App.appModule.repo,
          DATASTORE       = App.appModule.dataStoreManager)
      })
  val achievementViewModel =
    viewModel<AchievementViewModel>(factory = viewModelFactory() { AchievementViewModel() })
  val emulatorViewModel =
    viewModel<EmulatorViewModel>(factory = viewModelFactory()
    { EmulatorViewModel() })

  val isLandscape = App.appModule.isLandscape()

  Row(modifier = MODIFIFER.fillMaxSize())
  {
    if (isLandscape && state.showBars)
    {
      NavBar(
        SELECTED_INDEX = state.selectedIndex,
        ON_NAVIGATE    = { VIEW_MODEL.onNavigate(it) })
    }

    Scaffold (
      topBar    =
        {
          if (state.showBars)
          {
            TopAppBar(
              title          =
                { CustomText(TEXT = stringResource(VIEW_MODEL.getDestinationTitle()) ) },
              actions        =
                {
                  IconButton(onClick = { VIEW_MODEL.toggleSeeInfo() })
                  {
                    Icon(
                      painter            = painterResource(R.drawable.github),
                      contentDescription = stringResource(R.string.check_info),
                      tint                = GameBoyColors.DarkGreen,
                      modifier            = Modifier.size(24.dp))
                  }
                },
              colors         = TopAppBarDefaults.topAppBarColors(containerColor = GameBoyColors.MediumGreen))
          }
        },

      bottomBar =
        {
          if (!isLandscape && state.showBars)
          {
            NavBar(
              MODIFIER       = Modifier
                .fillMaxWidth(),
              SELECTED_INDEX = state.selectedIndex,
              ON_NAVIGATE    = { VIEW_MODEL.onNavigate(it)
              })
          }
        },

      modifier  = Modifier.weight(1f),
      snackbarHost = { SnackbarHost(hostState = SNACK) }
    )
    { innerPadding ->
      val navController       = rememberNavController()
      val navigator           = App.appModule.navigator

      ObserveAsEvents(navigator.navigationAction)
      { action ->
        when(action)
        {
          is NavigationAction.Navigate          -> navController.navigate(action.DESTINATION)  {action.OPTIONS(this) }
          is NavigationAction.PopBackStack      ->
          {
            if (action.DESTINATION != null) navController.popBackStack(action.DESTINATION, action.INCLUSIVE)
            else                            navController.popBackStack()
          }
          is NavigationAction.ClearBackStack    ->
          {
            navController.navigate(action.DESTINATION)
            {
              popUpTo(0) { inclusive = true}
              launchSingleTop = true
            }
          }
          is NavigationAction.NavigateSingleTop -> { navController.navigate(action.DESTINATION) { launchSingleTop = true } }
          is NavigationAction.PopUpTo           ->
          {
            navController.navigate(action.DESTINATION)
            {
              popUpTo(action.DESTINATION) { inclusive = action.INCLUSIVE}
              launchSingleTop = true
            }
          }
          is NavigationAction.Replace           ->
          {
            navController.popBackStack()
            navController.navigate(action.DESTINATION)
          }
          NavigationAction.NavigateBack         -> navController.navigateUp()
        }
      }


      NavHost(
        navController    = navController,
        startDestination = navigator.startDestination,
        modifier         = Modifier.padding(innerPadding))
      {
        composable<Destination.Home>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
          HomeScreen(
            VIEW_MODEL = gamesViewModel,
            MODIFIFER  = Modifier.fillMaxSize())
        }
        composable<Destination.Collections>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
          CollectionsScreen(
            VIEW_MODEL = collectionsViewModel,
            MODIFIFER  = Modifier.fillMaxSize())
        }
        composable<Destination.Search>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
          SearchScreen(
            VIEW_MODEL = gamesViewModel,
            MODIFIFER  = Modifier.fillMaxSize())
        }
        composable<Destination.Settings>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
          SettingsScreen(
            VIEW_MODEL = settingsViewModel,
            MODIFIFER  = Modifier.fillMaxSize())
        }
        composable<Destination.Achievements>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
          AchievementScreen(
            VIEW_MODEL = achievementViewModel)
        }
        composable<Destination.Emulator>
        {
          LaunchedEffect(Unit) { VIEW_MODEL.showBars(false) }
          val args = it.toRoute<Destination.Emulator>()
          EmulatorScreen(
            MODIFIER   = Modifier.fillMaxSize(),
            URI        = args.URI,
            VIEW_MODEL = emulatorViewModel)
        }
      }

      if (state.showInfoDialog)
      {
        var expandedIndex by remember { mutableIntStateOf(-1) }
        val faqs = listOf(
          "How do I add a game?" to "1. Copy your .gb or .gbc files into a folder on your device.\n2. Ensure they are NOT in a zip file.\n3. Go to Settings in this app.\n4. Tap 'Select Directory' and choose your ROMs folder.\n5. Wait for the scan to finish and your games will appear on the Home screen.",
          "Is this app open source?" to "Yes! Pocket Pixel is completely open source. You can view, audit, or contribute to the source code on GitHub using the link below.",
          "Are there any ads?" to "Never. Pocket Pixel is and will always be 100% ad-free. No interruptions, no tracking, just pure retro gaming.",
          "What about my data privacy?" to "Your privacy is paramount. No data ever leaves your device. Everything—including your game saves, screenshots, and settings—is stored locally on your phone.",
          "Audio is a bit glitchy, how do I fix?" to "Go to settings -> Audio and turn the sliders down (especially Noise) and use your device's audio buttons to increase volume. ",
          "How do I use a game controller?" to "1. Connect your controller via Bluetooth or USB.\n2. Go to Settings.\n3. Tap on any detected button or axis under 'Current Bindings'.\n4. Press the physical button on your controller to map it.",
          "Does this support piracy?" to "No. You are responsible for acquiring your games fairly. We recommend scanning physical cartridges you own or playing homebrew games from sites like itch.io.",
          "How do I take a screenshot?" to "While playing, open the In-game Settings and select 'Screenshot' under the Misc tab. You can view them by tapping 'View Screenshots' in the main Settings menu.",
          "What is Deferred Saving?" to "Deferred saving is a performance feature. Instead of pausing the game to save data immediately, it waits until you exit the emulator. This prevents minor stutters during gameplay.",
          "How do I use Save States?" to "Open the In-game Settings while playing. Under the 'States' tab, you can select one of 5 slots to Save or Load your exact progress at any moment.",
          "Can I change the colors?" to "Yes! Go to Settings and look for the 'Visual' section. You can choose from classic Game Boy palettes or apply shaders like 'CRT' or 'LCD' for a more authentic feel.",
          "How do I delete a save file?" to "Long-press any game card on the Home screen to open the action menu. From there, you can select 'Delete Save File' to clear the battery RAM (SRAM) for that game.",
          "How do I stop the controls from disappearing" to "Turn off Immersive Mode in settings."
        )

        BasicAlertDialog(
          onDismissRequest = { VIEW_MODEL.toggleSeeInfo() },
          modifier         = Modifier
            .fillMaxWidth(0.95f)
            .background(GameBoyColors.DarkGreen)
            .border(4.dp, GameBoyColors.Green, RectangleShape),
          content          =
            {
              Column(modifier = Modifier.padding(16.dp)) {
                Row(
                  modifier = Modifier.fillMaxWidth(),
                  horizontalArrangement = Arrangement.SpaceBetween,
                  verticalAlignment = Alignment.CenterVertically
                ) {
                  CustomText("FAQ", FONT_SIZE = 20, MODIFIER = Modifier.padding(8.dp))
                  CustomButton(
                    ON_CLICK = { VIEW_MODEL.toggleSeeInfo() },
                    MODIFIER = Modifier.width(60.dp),
                    COLOR = GameBoyColors.MediumGreen
                  ) {
                    CustomText("X", FONT_SIZE = 18, MODIFIER = Modifier.padding(4.dp))
                  }
                }

                HorizontalDivider(color = GameBoyColors.MediumGreen, thickness = 1.dp, modifier = Modifier.padding(vertical = 8.dp))

                LazyColumn(
                  modifier = Modifier.heightIn(max = 450.dp),
                  verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                  items(faqs.size) { index ->
                    FAQItemView(
                      QUESTION = faqs[index].first,
                      ANSWER = faqs[index].second,
                      IS_EXPANDED = expandedIndex == index,
                      ON_TOGGLE = { expandedIndex = if (expandedIndex == index) -1 else index }
                    )
                  }
                  
                  item {
                    Spacer(Modifier.height(8.dp))
                    CustomButton(
                      ON_CLICK = {
                        val githubUrl = "https://github.com/Asher-Ul-Haque/Pocket-Pixel"
                        val intent    = Intent(Intent.ACTION_VIEW, Uri.parse(githubUrl)).apply { addFlags(Intent.FLAG_ACTIVITY_NEW_TASK) }
                        context.startActivity(intent)
                      },
                      MODIFIER = Modifier.fillMaxWidth().padding(top = 12.dp)
                    ) {
                      Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.padding(8.dp)) {
                        Icon(painterResource(R.drawable.github), null, tint = GameBoyColors.DarkGreen, modifier = Modifier.size(20.dp))
                        Spacer(Modifier.width(8.dp))
                        CustomText(stringResource(R.string.GITHUB), COLOR = GameBoyColors.DarkGreen, MODIFIER = Modifier.padding(0.dp))
                      }
                    }
                  }
                }
              }
            }
        )
      }
    }
  }
}