package org.just_somebody.pocket_pixel.browseScreen.presentation

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import org.just_somebody.pocket_pixel.core.theme.GameBoyColors
import org.just_somebody.pocket_pixel.core.theme.PokeFontFamily

@Composable
fun temp(modifier: Modifier = Modifier)
{
  Box(contentAlignment = Alignment.Center)
  {
    Text(
      modifier = Modifier.padding(16.dp),
      text = "Coming Soon",
      color = GameBoyColors.LightGreen,
      fontSize = 64.sp,
      fontFamily = PokeFontFamily(),
    )
  }}