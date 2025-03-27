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
import org.just_somebody.pocket_pixel.EmulatorScreen
import org.just_somebody.pocket_pixel.browseScreen.data.ScreensToInt
import org.just_somebody.pocket_pixel.browseScreen.data.intToScreen
import org.just_somebody.pocket_pixel.core.Screens
import org.just_somebody.pocket_pixel.core.isLandscape
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.depInj.getMenuItems
import org.just_somebody.pocket_pixel.depInj.getNavController
import org.just_somebody.pocket_pixel.splashScreen.presentation.SplashScreen


fun navigate(SCREEN : ScreensToInt, NAV_CONTROLLER : NavController)
{
  println("Navigating");
  when (SCREEN)
  {
    ScreensToInt.Search     -> NAV_CONTROLLER.navigate(Screens.SearchPage);
    ScreensToInt.Explore    -> NAV_CONTROLLER.navigate(Screens.ExplorePage);
    ScreensToInt.Favorites  -> NAV_CONTROLLER.navigate(Screens.FavoritesPage);
    ScreensToInt.Me         -> NAV_CONTROLLER.navigate(Screens.MePage);
    ScreensToInt.Downloads  -> NAV_CONTROLLER.navigate(Screens.DownloadsPage);
  }
}


@Composable
fun MainScreen(
  MODIFIER  : Modifier = Modifier)
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
            startDestination  = Screens.ExplorePage
          )
          {
            composable<Screens.MePage>            {         temp();         }
            composable<Screens.ExplorePage>       {     EmulatorScreen()    }
            composable<Screens.FavoritesPage>     {         temp();         }
            composable<Screens.SearchPage>        {         temp();         }
            composable<Screens.DownloadsPage>     {         temp();         }
          }
        }
      }
    }
  )
}
