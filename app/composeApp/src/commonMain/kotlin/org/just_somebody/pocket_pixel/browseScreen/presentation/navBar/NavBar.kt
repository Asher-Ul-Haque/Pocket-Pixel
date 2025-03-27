import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.layout.wrapContentWidth
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.vector.rememberVectorPainter
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import org.jetbrains.compose.resources.DrawableResource
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.browseScreen.presentation.navBar.NavItem
import org.just_somebody.pocket_pixel.core.isLandscape
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily

@Composable
fun NavBar(
  ITEMS           : List<NavItem>,
  SELECTED_INDEX  : Int,
  ON_NAVIGATE     : (Int) -> Unit,
  MODIFIER        : Modifier = Modifier
)
{
  if (isLandscape())
  {
    Column(
      modifier            = MODIFIER
                            .fillMaxHeight()
                            .wrapContentWidth()
                            .background(GameBoyColors.MediumGreen)
                            .padding(vertical = 8.dp),
      verticalArrangement = Arrangement.SpaceEvenly,
      horizontalAlignment = Alignment.CenterHorizontally
    ) { NavList(ITEMS, SELECTED_INDEX, ON_NAVIGATE) }
  }
  else
  {
    Row(
      modifier              = MODIFIER
                              .fillMaxWidth()
                              .wrapContentHeight()
                              .background(GameBoyColors.MediumGreen)
                              .padding(horizontal = 8.dp),
      horizontalArrangement = Arrangement.SpaceEvenly,
      verticalAlignment     = Alignment.CenterVertically
    ) { NavList(ITEMS, SELECTED_INDEX, ON_NAVIGATE) }
  }
}

@Composable
private fun NavList(
  ITEMS           : List<NavItem>,
  SELECTED_INDEX  : Int,
  ON_NAVIGATE     : (Int) -> Unit
)
{
  ITEMS.forEachIndexed ()
  { index, item ->
    val isSelected = (index == SELECTED_INDEX)

    Box(
      modifier          = Modifier
                          .size(width = 72.dp, height = 72.dp)
                          .clickable { ON_NAVIGATE(index) }
                          .background(
                            color = if (isSelected) GameBoyColors.Green.copy(alpha = 0.2f)
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
          val icon =  if (isSelected) item.selectedIcon
                      else            item.unselectedIcon;
          Icon(
            painter = when (icon) {
              is DrawableResource         -> painterResource(icon)
              is ImageVector              -> rememberVectorPainter(image = icon) // Material Icon
              else                        -> throw IllegalArgumentException("Invalid icon type")
            },
            contentDescription  = item.title,
            tint                = GameBoyColors.DarkGreen,
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
                  fontFamily  = PokeFontFamily(),
                  fontSize    = 16.sp,
                )
              }
            }
          }
        }

        Spacer(modifier = Modifier.height(4.dp))

        Text(
          text        = item.title,
          fontFamily  = PokeFontFamily(),
          fontSize    = 20.sp,
          color       = GameBoyColors.LightGreen,
          softWrap    = false,
          maxLines    = 1
        )
      }
    }
  }
}
