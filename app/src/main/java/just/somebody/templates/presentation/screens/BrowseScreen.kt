package just.somebody.templates.presentation.screens

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.viewModels.BrowseViewModel
import just.somebody.templates.presentation.viewModels.ScreenAViewModel
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
  val selectedIndex by VIEW_MODEL.selectedIndex.collectAsState()
  Scaffold (
    topBar    =
    {
      TopAppBar(
        title          =
        { CustomText(TEXT = stringResource(R.string.app_name) ) },
        actions        =
        {
          IconButton(onClick = { TODO("Make the icon") })
          {
            Icon(
              painter            = painterResource(R.drawable.github),
              contentDescription = "Check information"
            )
          }

          IconButton(onClick = { VIEW_MODEL.goToSettings() })
          {
            Icon(
              painter            = painterResource(R.drawable.github),
              contentDescription = "Change Settings"
            )
          }
        },
        colors         = TopAppBarDefaults.topAppBarColors(containerColor = GameBoyColors.DarkGreen)
      )
    },

    bottomBar =
    {
      NavBar(
        MODIFIER       = Modifier
          .padding(bottom = 4.dp)
          .fillMaxWidth(),
        SELECTED_INDEX = selectedIndex,
        ON_NAVIGATE    = { VIEW_MODEL.onNavigate(it)
        }
      )
    },

    modifier  = MODIFIFER,

    snackbarHost = { SnackbarHost(hostState = SNACK) }
  )
  { innerPadding ->
    val navController = rememberNavController()
    val navigator     = App.appModule.navigator
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
        ScreenA(
          VIEW_MODEL = viewModel<ScreenAViewModel>(factory = viewModelFactory()
          { ScreenAViewModel(App.appModule.navigator) }),
        )
      }
      composable<Destination.Favorites> { TODO("Make favorites screen") }
      composable<Destination.Search>    { TODO("Make search screen") }
      composable<Destination.Server>    { TODO("Make server screen") }
    }
  }
}