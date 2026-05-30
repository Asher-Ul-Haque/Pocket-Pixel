package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.App
import just.somebody.templates.R
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily


data class NavItem(
  val title          : String,
  val unselectedIcon : Int,
  val selectedIcon   : Int,
  val badgeCount     : Int? = null,
)

@Composable
fun NavBar(
  SELECTED_INDEX  : Int,
  ON_NAVIGATE     : (Int) -> Unit,
  MODIFIER        : Modifier = Modifier
)
{
  if (App.appModule.isLandscape())
  {
    Column (
      modifier            = MODIFIER
        .fillMaxHeight()
        .wrapContentWidth()
        .background(GameBoyColors.MediumGreen),
      horizontalAlignment =  Alignment.CenterHorizontally,
      verticalArrangement = Arrangement.SpaceEvenly
    ) { NavList(SELECTED_INDEX, ON_NAVIGATE) }
  }
  else
  {
    Row (
      modifier            = MODIFIER
        .fillMaxWidth()
        .wrapContentHeight()
        .background(GameBoyColors.MediumGreen),
      verticalAlignment     = Alignment.CenterVertically,
      horizontalArrangement = Arrangement.SpaceEvenly
    ) { NavList(SELECTED_INDEX, ON_NAVIGATE) }
  }
}

@Composable
private fun NavList(
  SELECTED_INDEX  : Int,
  ON_NAVIGATE     : (Int) -> Unit
)
{
  val navItems = listOf<NavItem>(
    NavItem(
      stringResource(R.string.HOME),
      R.drawable.gamepad,
      R.drawable.gamepad
    ),

    NavItem(
      stringResource(R.string.FAV),
      R.drawable.heart,
      R.drawable.heart
    ),

    NavItem(
      stringResource(R.string.SEARCH),
      R.drawable.search,
      R.drawable.search
    ),
  )

  navItems.forEachIndexed ()
  { index, item ->
    val isSelected = (index == SELECTED_INDEX)

    Box(
      modifier          = Modifier
        .size(width = 72.dp, height = 64.dp)
        .clickable { ON_NAVIGATE(index) }
        .background(
          color =
            if (isSelected) GameBoyColors.Green.copy(alpha = 0.2f)
            else            Color.Transparent,
          shape = RectangleShape
        )
        .padding(8.dp),
      contentAlignment = Alignment.Center
    )
    {
      Column(horizontalAlignment = Alignment.CenterHorizontally)
      {
        Box(modifier = Modifier.size(24.dp))
        {
          val icon =
            if (isSelected) item.selectedIcon
            else            item.unselectedIcon;
          Icon(
            painter             = painterResource(icon),
            contentDescription  = item.title,
            tint                =
              if (isSelected) GameBoyColors.LightGreen
              else            GameBoyColors.DarkGreen,
            modifier            = Modifier.size(24.dp)
          )
          if (item.badgeCount != null)
          {
            Box(
              modifier          = Modifier
                .size(14.dp)
                .align(Alignment.TopEnd)
                .offset(
                  x = (8).dp,
                  y = (-4).dp)
                .background(
                  color = GameBoyColors.LightGreen,
                  shape = RectangleShape),
              contentAlignment  = Alignment.Center
            )
            {
              if (item.badgeCount > 0)
              {
                Text(
                  text        = item.badgeCount.toString(),
                  color       = GameBoyColors.DarkGreen,
                  fontFamily  = MinecraftFontFamily,
                  fontSize    = 8.sp,
                )
              }
            }
          }
        }

        Spacer(modifier = Modifier.height(4.dp))

        Text(
          text        = item.title,
          fontFamily  = MinecraftFontFamily,
          fontSize    = 10.sp,
          color       = GameBoyColors.LightGreen,
          softWrap    = false,
          maxLines    = 1
        )
      }
    }
  }
}