package just.somebody.templates.presentation.widgets

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
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.LineHeightStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.R
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.MinecraftFontFamily

/**
 * A stylized search text input element utilizing a custom monospaced typography aesthetic.
 *
 * It embeds hardware keyboard integration mapping the software [ImeAction.Search] hook to a custom execution dispatch
 * pipeline while explicitly configuring overriding bounds for global selection highlights and system soft-key behaviors.
 *
 * @param SEARCH_QUERY The dynamic live state sequence string rendered in the editable text body region.
 * @param ON_SEARCH_QUERY_CHANGE Functional stream capture pipeline handling ongoing text alteration signals.
 * @param ON_SEARCH_TRIGGER Dispatch action fired when the soft keyboard confirmation key triggers processing flags.
 * @param MODIFIER [Modifier] used to establish positional alignments, dimension constraints, or border padding.
 */
@Composable
fun SearchBar(
  SEARCH_QUERY            : String,
  ON_SEARCH_QUERY_CHANGE  : (String) -> Unit,
  ON_SEARCH_TRIGGER       : () -> Unit,
  MODIFIER                : Modifier = Modifier)
{
  // - - - animation
  val keyboardController = LocalSoftwareKeyboardController.current

  CompositionLocalProvider(
    LocalTextSelectionColors provides TextSelectionColors(
      handleColor     = GameBoyColors.LightGreen,
      backgroundColor = GameBoyColors.MediumGreen)
                          )
  {
    OutlinedTextField(
      value           = SEARCH_QUERY,
      onValueChange   = ON_SEARCH_QUERY_CHANGE,
      shape           = RectangleShape,
      colors          = OutlinedTextFieldDefaults.colors(
        focusedBorderColor   = GameBoyColors.Green,
        unfocusedBorderColor = GameBoyColors.Green,
        cursorColor          = GameBoyColors.DarkGreen),
      keyboardOptions = KeyboardOptions(
        imeAction     = ImeAction.Search,
        keyboardType  = KeyboardType.Text),
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
            fontSize    = 16.sp,
            fontFamily  = MinecraftFontFamily,)
        },
      textStyle = TextStyle(
        color           = GameBoyColors.DarkGreen,
        fontSize        = 16.sp,
        fontFamily      = MinecraftFontFamily,
        lineHeight      = 28.sp,
        lineHeightStyle = LineHeightStyle(
          alignment = LineHeightStyle.Alignment.Center,
          trim      = LineHeightStyle.Trim.None)
                           ),
      leadingIcon     =
        {
          Icon(
            painter             = painterResource(R.drawable.search),
            contentDescription  = null,
            tint                = GameBoyColors.DarkGreen,
            modifier            = Modifier.size(24.dp))
        },
      singleLine = true,
      modifier   = MODIFIER
        .minimumInteractiveComponentSize()
        .background(GameBoyColors.Green))
  }
}