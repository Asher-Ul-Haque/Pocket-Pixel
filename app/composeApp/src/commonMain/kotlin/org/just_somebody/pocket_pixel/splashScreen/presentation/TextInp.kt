import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.text.selection.LocalTextSelectionColors
import androidx.compose.foundation.text.selection.TextSelectionColors
import androidx.compose.material3.* // Import Material3 components
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily

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
                                fontFamily  = PokeFontFamily()
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
            fontFamily  = PokeFontFamily(),
          )
        },
    )
  }
  return text
}