package org.just_somebody.pocket_pixel

import androidx.compose.runtime.Composable
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import org.just_somebody.pocket_pixel.browseScreen.presentation.MainScreen
import org.just_somebody.pocket_pixel.browseScreen.presentation.temp
import org.just_somebody.pocket_pixel.core.Screens
import org.just_somebody.pocket_pixel.depInj.getNavController
import org.just_somebody.pocket_pixel.splashScreen.presentation.SplashScreen

@Composable
fun App()
{
  NavHost(
    navController     = getNavController(),
    startDestination  = Screens.SplashScreen
  )
  {
    composable<Screens.SplashScreen>      { SplashScreen(); }
    composable<Screens.MainScreen>        {  MainScreen();  }
    composable<Screens.MePage>            {     temp();     }
    composable<Screens.ExplorePage>       {     temp();     }
    composable<Screens.FavoritesPage>     {     temp();     }
    composable<Screens.SearchPage>        {     temp();     }
    composable<Screens.DownloadsPage>     {     temp();     }
  }
}