package org.just_somebody.pocket_pixel.browseScreen.data
import org.just_somebody.pocket_pixel.browseScreen.presentation.navBar.NavItem
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.searchIcon
import pocketpixel.composeapp.generated.resources.user
import pocketpixel.composeapp.generated.resources.moonStars
import pocketpixel.composeapp.generated.resources.heart
import pocketpixel.composeapp.generated.resources.download



enum class ScreensToInt(val SCREEN : Int)
{
  Search    (0),
  Explore   (1),
  Favorites (2),
  Me        (3),
  Downloads (4)
}

fun intToScreen(INT : Int) : ScreensToInt
{
  when (INT % 5)
  {
    0 -> return ScreensToInt.Search;
    1 -> return ScreensToInt.Explore;
    2 -> return ScreensToInt.Favorites;
    3 -> return ScreensToInt.Me;
    4 -> return ScreensToInt.Downloads
  }
  return ScreensToInt.Explore;
}