package just.somebody.templates

import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.animation.OvershootInterpolator
import android.view.KeyEvent
import android.view.MotionEvent
import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.core.content.ContextCompat
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarResult
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.core.animation.doOnEnd
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.viewmodel.compose.viewModel
import just.somebody.templates.ui.theme.TemplateTheme
import just.somebody.templates.appModule.storage.StorageInitializer
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.effects.SoundEffect
import just.somebody.templates.presentation.screens.BrowseScreen
import just.somebody.templates.presentation.screens.Destination
import just.somebody.templates.presentation.viewModels.BrowseViewModel
import just.somebody.templates.presentation.viewModels.SplashViewModel
import just.somebody.templates.presentation.viewModels.viewModelFactory
import just.somebody.templates.domain.PauseTrigger
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity()
{
  private val requestPermissionLauncher = registerForActivityResult(
    ActivityResultContracts.RequestPermission()
  ) { isGranted: Boolean ->
    if (isGranted) {
      // Permission granted
    } else {
      // Permission denied
    }
  }

  override fun onCreate(savedInstanceState: Bundle?)
  {
    super.onCreate(savedInstanceState)
    
    // - - - Centralized Storage Initialization: Triggered whenever ROMs URI is updated or loaded
    lifecycleScope.launch {
        App.appModule.dataStoreManager.settingsFlow.collect { settings ->
            if (settings.externalUris.containsKey("GAME_BOY_ROMS")) {
                StorageInitializer.initialize(this@MainActivity)
            }
        }
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
      if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
        requestPermissionLauncher.launch(Manifest.permission.POST_NOTIFICATIONS)
      }
    }

    val splashViewModel by viewModels<SplashViewModel>()

    installSplashScreen().apply()
    {
      //setKeepOnScreenCondition { !splashViewModel.isReady.value }

      setOnExitAnimationListener()
      { splash ->
        // - - - Play splash sound
        SoundController.play(SoundEffect.Splash)

				splash.iconView.let()
				{ iconView ->
					// - - - Scale X animation
					val scaleX = ObjectAnimator.ofFloat(iconView, View.SCALE_X, 1.0f, 0.4f).apply { duration = 250L }
					// - - - Scale Y animation
					val scaleY = ObjectAnimator.ofFloat(iconView, View.SCALE_Y, 1.0f, 0.4f).apply { duration = 250L }
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

  override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
    if (App.appModule.gameControllerManager.handleKeyEvent(event)) {
      return true
    }
    return super.onKeyDown(keyCode, event)
  }

  override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
    if (App.appModule.gameControllerManager.handleKeyEvent(event)) {
      return true
    }
    return super.onKeyUp(keyCode, event)
  }

  override fun onGenericMotionEvent(event: MotionEvent): Boolean {
    if (App.appModule.gameControllerManager.handleMotionEvent(event)) {
      return true
    }
    return super.onGenericMotionEvent(event)
  }

  override fun onStop()
  {
    super.onStop()
    App.appModule.gameBoy.pauseEmulator(PauseTrigger.FOCUS)
  }

  override fun onRestart()
  {
    super.onRestart()
    App.appModule.gameBoy.resumeEmulator(PauseTrigger.FOCUS)
  }

  override fun onDestroy()
  {
    super.onDestroy()
    App.appModule.gameBoy.stopEmulator()
  }

}