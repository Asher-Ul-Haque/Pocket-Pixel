package just.somebody.templates.presentation.widgets

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.selection.LocalTextSelectionColors
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.minimumInteractiveComponentSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import just.somebody.templates.ui.theme.GameBoyColors
import just.somebody.templates.ui.theme.PokeFontFamily

@Composable
fun TextInp(
  PLACEHOLDER : String,
  MODIFIER    : Modifier  = Modifier,
  PASSWORD    : Boolean   = false,
  MAX_CHAR    : Int       = 25
): String
{
  var text by remember { mutableStateOf("") }
  CompositionLocalProvider(
    LocalTextSelectionColors provides TextSelectionColors(
      handleColor     = GameBoyColors.LightGreen,
      backgroundColor = GameBoyColors.MediumGreen
    )
  )
  {
    OutlinedTextField(
      value                 = text.subSequence(0, minOf(text.length, MAX_CHAR)).toString(),
      onValueChange         = { text = it },
      keyboardOptions       = if (PASSWORD) KeyboardOptions(keyboardType = KeyboardType.Password)
      else          KeyboardOptions(keyboardType = KeyboardType.Text),
      visualTransformation  = if (PASSWORD) PasswordVisualTransformation()
      else          VisualTransformation.None,
      textStyle             = TextStyle(
        color       = GameBoyColors.DarkGreen,
        fontSize    = 36.sp,
        fontFamily  = PokeFontFamily
      ),
      modifier              = MODIFIER
        .minimumInteractiveComponentSize()
        .padding(8.dp)
        .background(GameBoyColors.Green),
      singleLine            = true,
      colors                = OutlinedTextFieldDefaults.colors(
        focusedBorderColor    = GameBoyColors.Green,
        unfocusedBorderColor  = GameBoyColors.Green,
        cursorColor           = GameBoyColors.DarkGreen,
      ),
      placeholder           =
      {
        Text(
          text        = PLACEHOLDER,
          color       = GameBoyColors.MediumGreen,
          fontSize    = 36.sp,
          fontFamily  = PokeFontFamily,
        )
      },
    )
  }
  return text
}