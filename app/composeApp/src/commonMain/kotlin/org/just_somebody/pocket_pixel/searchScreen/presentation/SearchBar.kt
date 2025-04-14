package org.just_somebody.pocket_pixel.searchScreen.presentation


import androidx.compose.foundation.background
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.selection.LocalTextSelectionColors
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.Icon
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.minimumInteractiveComponentSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.platform.LocalSoftwareKeyboardController
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import org.jetbrains.compose.resources.painterResource
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily
import pocketpixel.composeapp.generated.resources.Res
import pocketpixel.composeapp.generated.resources.searchIcon

@Composable
fun SearchBar(
  SEARCH_QUERY            : String,
  ON_SEARCH_QUERY_CHANGE  : (String) -> Unit,
  ON_SEARCH_TRIGGER       : () -> Unit,
  MODIFIER                : Modifier = Modifier
)
{
  val keyboardController = LocalSoftwareKeyboardController.current

  CompositionLocalProvider(
    LocalTextSelectionColors provides TextSelectionColors(
      handleColor     = GameBoyColors.LightGreen,
      backgroundColor = GameBoyColors.MediumGreen
    )
  )
  {
    OutlinedTextField(
      value           = SEARCH_QUERY,
      onValueChange   = ON_SEARCH_QUERY_CHANGE,
      shape           = RectangleShape,
      colors          = OutlinedTextFieldDefaults.colors(
        focusedBorderColor    = GameBoyColors.Green,
        unfocusedBorderColor  = GameBoyColors.Green,
        cursorColor           = GameBoyColors.DarkGreen,
      ),
      keyboardOptions = KeyboardOptions(
        imeAction     = ImeAction.Search,
        keyboardType  = KeyboardType.Text
      ),
      keyboardActions = KeyboardActions(
        onSearch =
          {
            ON_SEARCH_TRIGGER()
            keyboardController?.hide()
          }
      ),
      placeholder     =
      {
        Text(
          text        = "Search...",
          color       = GameBoyColors.MediumGreen,
          fontSize    = 24.sp,
          fontFamily  = PokeFontFamily(),
        )
      },
      textStyle       = TextStyle(
        color       = GameBoyColors.DarkGreen,
        fontSize    = 24.sp,
        fontFamily  = PokeFontFamily()
      ),
      leadingIcon     =
      {
        Icon(
          painter             = painterResource(Res.drawable.searchIcon),
          contentDescription  = null,
          tint                = GameBoyColors.DarkGreen,
          modifier            = Modifier.size(24.dp)
        )
      },
      singleLine      = true,
      modifier = MODIFIER
        .minimumInteractiveComponentSize()
        .background(GameBoyColors.Green)
    )
  }
}