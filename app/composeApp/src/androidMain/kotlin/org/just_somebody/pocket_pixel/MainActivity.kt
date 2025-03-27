package org.just_somebody.pocket_pixel

import TextInp
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import org.just_somebody.pocket_pixel.splashScreen.depInj.setContext


class MainActivity : ComponentActivity()
{
  override fun onCreate(savedInstanceState: Bundle?)
  {
    enableEdgeToEdge()
    super.onCreate(savedInstanceState)
    setContext(applicationContext);
    setContent { App(); }
  }
}

@Preview
@Composable
fun prev(modifier: Modifier = Modifier)
{
  TextInp("")
}