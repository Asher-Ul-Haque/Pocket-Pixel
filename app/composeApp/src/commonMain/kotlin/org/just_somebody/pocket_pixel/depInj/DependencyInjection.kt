package org.just_somebody.pocket_pixel.depInj

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.sharp.AccountBox
import androidx.compose.material.icons.sharp.AccountCircle
import androidx.compose.material.icons.sharp.Search
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.navigation.NavHostController
import androidx.navigation.compose.rememberNavController
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.browseScreen.presentation.navBar.NavItem
import org.just_somebody.pocket_pixel.core.isLandscape
import org.just_somebody.pocket_pixel.splashScreen.data.SplashScreenNetworkCalls
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.download
import pocketpixel.composeapp.generated.resources.heart
import pocketpixel.composeapp.generated.resources.moonStars
import pocketpixel.composeapp.generated.resources.searchIcon
import pocketpixel.composeapp.generated.resources.user

val items          : List<NavItem>            = listOf(
  NavItem(
    title           = "Search",
    unselectedIcon  = Res.drawable.searchIcon,
    selectedIcon    = Res.drawable.searchIcon,
    badgeCount      = null,
  ),
  NavItem(
    title           = "Explore",
    unselectedIcon  = Res.drawable.moonStars,
    selectedIcon    = Res.drawable.moonStars,
    badgeCount      = 3,
  ),
  NavItem(
    title           = "Favorites",
    unselectedIcon  = Res.drawable.heart,
    selectedIcon    = Res.drawable.heart,
    badgeCount      = null,
  ),
  NavItem(
    title           = "Me",
    unselectedIcon  = Res.drawable.user,
    selectedIcon    = Res.drawable.user,
    badgeCount      = null,
  ),
  NavItem(
    title           = "Downloads",
    unselectedIcon  = Res.drawable.download,
    selectedIcon    = Res.drawable.download,
    badgeCount      = null,
  )
);


private val splashNetCalls : SplashScreenNetworkCalls = SplashScreenNetworkCalls();
private var navController  : NavHostController?       = null;

fun getSplashNetworkCalls() : SplashScreenNetworkCalls
{
  return splashNetCalls;
}

@Composable
fun getNavController()  : NavHostController
{
  if (navController == null) navController = rememberNavController()
  return navController as NavHostController;
}

@Composable
fun getMenuItems()      : List<NavItem>
{
  return items.subList(
    fromIndex = 0,
    toIndex   =
        if (isLandscape())  items.size - 1;
        else                items.size
  )
}