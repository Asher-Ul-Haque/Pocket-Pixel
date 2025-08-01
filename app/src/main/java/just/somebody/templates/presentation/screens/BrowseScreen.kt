package just.somebody.templates.presentation.screens

import android.content.Intent
import android.net.Uri
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.BasicAlertDialog
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.appModule.navigation.NavigationAction
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.viewModels.BrowseViewModel
import just.somebody.templates.presentation.viewModels.EmulatorViewModel
import just.somebody.templates.presentation.viewModels.GamesViewModel
import just.somebody.templates.presentation.viewModels.LinkCableViewModel
import just.somebody.templates.presentation.viewModels.SettingsViewModel
import just.somebody.templates.presentation.viewModels.viewModelFactory
import just.somebody.templates.presentation.widgets.CustomText
import just.somebody.templates.presentation.widgets.NavBar
import just.somebody.templates.ui.theme.GameBoyColors

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BrowseScreen(
  SNACK      : SnackbarHostState,
  VIEW_MODEL : BrowseViewModel,
  MODIFIFER  : Modifier = Modifier
)
{
  val state by VIEW_MODEL.browseState.collectAsState()

  val gamesViewModel =
    viewModel<GamesViewModel>(factory = viewModelFactory()
    { GamesViewModel(App.appModule.repo)
    })
  val settingsViewModel =
    viewModel<SettingsViewModel>(
      factory = viewModelFactory()
        {
          SettingsViewModel(
          REPO            = App.appModule.repo,
          DATASTORE       = App.appModule.dataStoreManager)
        }
    )
  val emulatorViewModel =
    viewModel<EmulatorViewModel>(factory = viewModelFactory()
    { EmulatorViewModel() })
  val linkCableViewModel =
    viewModel<LinkCableViewModel>(factory = viewModelFactory()
    { LinkCableViewModel() })

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
                contentDescription = "Check information",
                tint                = GameBoyColors.DarkGreen,
                modifier            = Modifier.size(24.dp)
              )
            }

            IconButton(onClick = { VIEW_MODEL.goToSettings(true) })
            {
              Icon(
                painter            = painterResource(R.drawable.settings),
                contentDescription = "Change Settings",
                tint                = GameBoyColors.DarkGreen,
                modifier            = Modifier.size(24.dp)
              )
            }
          },
          colors         = TopAppBarDefaults.topAppBarColors(containerColor = GameBoyColors.MediumGreen)
        )
      }
    },

    bottomBar =
    {
      if (state.showBars)
      {
        NavBar(
          MODIFIER       = Modifier
            .fillMaxWidth(),
          SELECTED_INDEX = state.selectedIndex,
          ON_NAVIGATE    = { VIEW_MODEL.onNavigate(it)
          }
        )
      }
    },

    modifier  = MODIFIFER,

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
      modifier         = Modifier.padding(innerPadding)
    )
    {
      composable<Destination.Home>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
        HomeScreen(
          VIEW_MODEL = gamesViewModel,
          MODIFIFER  = Modifier.fillMaxSize()
        )
      }
      composable<Destination.Favorites>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
        FavoriteScreen(
          VIEW_MODEL = gamesViewModel,
          MODIFIFER  = Modifier.fillMaxSize()
        )
      }
      composable<Destination.Search>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
        SearchScreen(
          VIEW_MODEL = gamesViewModel,
          MODIFIFER  = Modifier.fillMaxSize()
        )
      }
      composable<Destination.Server>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
        ServerScreen(Modifier.fillMaxSize())
      }
      composable<Destination.LinkCable>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(true) }
        LinkCableScreen(linkCableViewModel, Modifier.fillMaxSize())
      }
      composable<Destination.Emulator>
      {
        LaunchedEffect(Unit) { VIEW_MODEL.showBars(false) }
        val args = it.toRoute<Destination.Emulator>()
        EmulatorScreen(
          MODIFIER   = Modifier.fillMaxSize(),
          URI        = args.URI,
          VIEW_MODEL = emulatorViewModel,
          LINK_CABLE = linkCableViewModel)
      }
    }

    if (state.showInfoDialog)
    {
      BasicAlertDialog(
        onDismissRequest = { VIEW_MODEL.toggleSeeInfo() },
        modifier         = Modifier.background(GameBoyColors.MediumGreen),
        content          =
        {
          Column (
            verticalArrangement = Arrangement.SpaceEvenly,
            horizontalAlignment = Alignment.CenterHorizontally
          )
          {
            CustomText(
              TEXT      = stringResource(R.string.HOW_TO),
              FONT_SIZE = 24,
              COLOR     = GameBoyColors.Green)
            CustomText(
              TEXT      = stringResource(R.string.USAGE),
              FONT_SIZE = 16,
              COLOR     = GameBoyColors.LightGreen)

            CustomText(
              TEXT      = stringResource(R.string.NOT_WORKING),
              FONT_SIZE = 24,
              COLOR     = GameBoyColors.Green)
            CustomText(
              TEXT      = stringResource(R.string.TROUBLESHOOT),
              FONT_SIZE = 16,
              COLOR     = GameBoyColors.LightGreen)

            CustomText(
              TEXT      = stringResource(R.string.LEGAL),
              FONT_SIZE = 24,
              COLOR     = GameBoyColors.Green)
            CustomText(
              TEXT      = stringResource(R.string.PIRACY),
              FONT_SIZE = 16,
              COLOR     = GameBoyColors.LightGreen)

            CustomText(
              TEXT     = stringResource(R.string.GITHUB),
              MODIFIER = Modifier.clickable ()
                {
                  val githubUrl = "https://github.com/Asher-Ul-Haque/Pocket-Pixel"
                  val intent    = Intent(Intent.ACTION_VIEW, Uri.parse(githubUrl)).apply { addFlags(Intent.FLAG_ACTIVITY_NEW_TASK) }
                  App.appModule.context.startActivity(intent)
                },
              COLOR   = Color.Blue
            )
          }
        })
    }

    if (state.showSettings)
    {
      SettingsScreen(
        VIEW_MODEL = settingsViewModel,
        ON_DISMISS = { VIEW_MODEL.goToSettings(false) }
      )
    }
  }
}