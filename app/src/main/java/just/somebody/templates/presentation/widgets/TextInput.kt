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
import just.somebody.templates.ui.theme.MinecraftFontFamily

/**
 * A specialized retro text entry layout that manages internal character sequence buffers.
 *
 * This component provides security masking options for password entries, enforces strict horizontal
 * length caps via inline character extraction math, and overrides text cursor highlight colors to
 * match the console's green theme palette. It leverages a direct return expression to instantly expose
 * its localized state buffer back up to parent components without requiring hoisted container variables.
 *
 * @param PLACEHOLDER Floating text string displayed inside the editable track when the input stream remains empty.
 * @param MODIFIER [Modifier] used to establish dimensional constraints, border offsets, or parent cell padding rules.
 * @param PASSWORD Toggles secure obscured display characters and configures a digital numeric mask keyboard type. Defaults to `false`.
 * @param MAX_CHAR Direct integer layout boundary threshold that hard-clips text length overflows. Defaults to `25`.
 * @return The live, continuous [String] sequence currently stored within the internal composition text buffer.
 */
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
        fontFamily  = MinecraftFontFamily
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
            fontFamily  = MinecraftFontFamily,
              )
        },
                     )
  }
  return text
}