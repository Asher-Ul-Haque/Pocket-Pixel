package org.just_somebody.pocket_pixel.browseScreen.presentation

import NavBar
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import org.just_somebody.pocket_pixel.emulatorScreen.presentation.EmulatorScreen
import org.just_somebody.pocket_pixel.browseScreen.data.ScreensToInt
import org.just_somebody.pocket_pixel.browseScreen.data.intToScreen
import org.just_somebody.pocket_pixel.cartridgeScreen.presentation.CartridgeUI
import org.just_somebody.pocket_pixel.core.Screens
import org.just_somebody.pocket_pixel.core.isLandscape
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.depInj.getMenuItems
import org.just_somebody.pocket_pixel.favoritesScreen.presentation.FavoritesScreen
import org.just_somebody.pocket_pixel.searchScreen.presentation.SearchScreen


fun navigate(SCREEN : ScreensToInt, NAV_CONTROLLER : NavController)
{
  when (SCREEN)
  {
    ScreensToInt.Search     -> NAV_CONTROLLER.navigate(Screens.SearchPage);
    ScreensToInt.Explore    -> NAV_CONTROLLER.navigate(Screens.ExplorePage);
    ScreensToInt.Favorites  -> NAV_CONTROLLER.navigate(Screens.FavoritesPage);
    ScreensToInt.Me         -> NAV_CONTROLLER.navigate(Screens.MePage);
    ScreensToInt.Downloads  -> NAV_CONTROLLER.navigate(Screens.DownloadsPage);
    ScreensToInt.Cartridge  -> NAV_CONTROLLER.navigate(Screens.CartridgePage)
  }
}


@Composable
fun MainScreen(MODIFIER  : Modifier = Modifier)
{
  var selectedIndex by remember { mutableStateOf(2) }
  val navController = rememberNavController()

  Scaffold(
    bottomBar =
    {
      if (!isLandscape())
      {
        NavBar(
          ITEMS           = getMenuItems(),
          SELECTED_INDEX  = selectedIndex,
          ON_NAVIGATE     = { selectedIndex = it; navigate(intToScreen(it), navController) },
          MODIFIER        = Modifier.fillMaxWidth().padding(bottom = 4.dp)
        )
      }
    },
    content =
    { paddingValues ->
      Row(
        modifier = Modifier
          .fillMaxSize()
          .background(GameBoyColors.DarkGreen)
          .padding(paddingValues)
      )
      {
        if (isLandscape())
        {
          NavBar(
            ITEMS           = getMenuItems(),
            SELECTED_INDEX  = selectedIndex,
            ON_NAVIGATE     = { selectedIndex = it; navigate(intToScreen(it), navController) },
            MODIFIER        = Modifier.fillMaxHeight()
          )
        }
        Column(
          modifier            = Modifier.fillMaxSize(),
          verticalArrangement = Arrangement.Center,
          horizontalAlignment = Alignment.CenterHorizontally
        )
        {
          NavHost(
            navController     = navController,
            startDestination  = Screens.FavoritesPage
          )
          {
            composable<Screens.MePage>            {         temp();         }
            composable<Screens.ExplorePage>       {    EmulatorScreen();    }
            composable<Screens.FavoritesPage>
            { FavoritesScreen(GO_TO_GAME = { navController.navigate(Screens.CartridgePage) }); }
            composable<Screens.SearchPage>
            { SearchScreen(GO_TO_GAME = { navController.navigate(Screens.CartridgePage) }); }
            composable<Screens.DownloadsPage>     {         temp();         }
            composable<Screens.CartridgePage>     {      CartridgeUI()      }
          }
        }
      }
    }
  )
}
