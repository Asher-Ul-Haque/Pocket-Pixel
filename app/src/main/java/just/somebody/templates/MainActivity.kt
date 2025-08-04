package just.somebody.templates

import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.os.Bundle
import android.view.View
import android.view.animation.OvershootInterpolator
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarResult
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.core.animation.doOnEnd
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.core.view.WindowCompat
import androidx.lifecycle.viewmodel.compose.viewModel
import just.somebody.pocketpixel.ui.theme.TemplateTheme
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import just.somebody.templates.presentation.screens.BrowseScreen
import just.somebody.templates.presentation.viewModels.BrowseViewModel
import just.somebody.templates.presentation.viewModels.SplashViewModel
import just.somebody.templates.presentation.viewModels.viewModelFactory
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity()
{
  override fun onCreate(savedInstanceState: Bundle?)
  {
    super.onCreate(savedInstanceState)

    val splashViewModel by viewModels<SplashViewModel>()

    installSplashScreen().apply()
    {
      setKeepOnScreenCondition { !splashViewModel.isReady.value }

      setOnExitAnimationListener()
      { splash ->
        // - - - Play custom sound effect (if needed)
        SoundController.play(SoundEffect.Splash)
        val iconView = splash.iconView
        // - - - Scale X animation
        val scaleX = ObjectAnimator.ofFloat(iconView, View.SCALE_X, 1.0f, 0.4f).apply { duration = 500L }
        // - - - Scale Y animation
        val scaleY = ObjectAnimator.ofFloat(iconView, View.SCALE_Y, 1.0f, 0.4f).apply { duration = 500L }
        // - - - fade out
        val fade = ObjectAnimator.ofFloat(iconView, View.ALPHA, 1f, 0f).apply { duration = 500L }

        // - - - Run animations together
        AnimatorSet().apply()
        {
          playTogether(scaleX, scaleY, fade)
          interpolator = OvershootInterpolator()
          doOnEnd { splash.remove() }
          start()
        }
      }
    }

    setContent ()
    {
      TemplateTheme()
      {
        val snackbarHostState = remember { SnackbarHostState() }
        val snackbarScope     = rememberCoroutineScope()
        ObserveAsEvents(
          FLOW = SnackbarController.events,
          KEY  = snackbarHostState)
        { event ->
          snackbarScope.launch ()
          {
            snackbarHostState.currentSnackbarData?.dismiss()
            val result = snackbarHostState.showSnackbar(
              message     = event.message,
              actionLabel = event.action?.name,
              duration    = event.duration
            )

            if (result == SnackbarResult.ActionPerformed)  event.action?.action?.invoke()
          }
        }

        val soundScope        = rememberCoroutineScope()
        ObserveAsEvents(FLOW = SoundController.effects)
        { event ->
          soundScope.launch ()
          {
            snackbarHostState.currentSnackbarData?.dismiss()
            SoundController.play(event)
          }
        }

        BrowseScreen(
          SNACK      = snackbarHostState,
          VIEW_MODEL = viewModel<BrowseViewModel>(factory = viewModelFactory { BrowseViewModel() }),
          MODIFIFER  = Modifier.fillMaxSize())
      }
    }
  }

  override fun onStop()
  {
    super.onStop()
    App.appModule.gameBoy.pauseEmulator()
  }

  override fun onRestart()
  {
    super.onRestart()
    App.appModule.gameBoy.resumeEmulator()
  }

  override fun onDestroy()
  {
    super.onDestroy()
    App.appModule.gameBoy.stopEmulator()
    App.appModule.gameBoy.flushSave()
  }
}