package just.somebody.templates

import android.animation.ObjectAnimator
import android.os.Bundle
import android.view.View
import android.view.animation.OvershootInterpolator
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.viewModels
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarResult
import androidx.compose.material3.Text
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.core.animation.doOnEnd
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute
import just.somebody.pocketpixel.ui.theme.TemplateTheme
import just.somebody.templates.presentation.effects.ObserveAsEvents
import just.somebody.templates.presentation.screens.Destination
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SoundController
import just.somebody.templates.presentation.screens.NavigationAction
import just.somebody.templates.presentation.screens.ScreenA
import just.somebody.templates.presentation.viewModels.ScreenAViewModel
import just.somebody.templates.presentation.viewModels.SplashViewModel
import just.somebody.templates.presentation.viewModels.viewModelFactory
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity()
{
  override fun onCreate(savedInstanceState: Bundle?)
  {
    super.onCreate(savedInstanceState)

    val splashViewModel by viewModels<SplashViewModel>()

    installSplashScreen().apply ()
    {
      setKeepOnScreenCondition   { !splashViewModel.isReady.value }
      setOnExitAnimationListener ()
      { splash ->
        val zoomX = ObjectAnimator.ofFloat(
          splash.iconView,
          View.SCALE_X,
          1.0f,
          0.0f
        )
        zoomX.interpolator  = OvershootInterpolator()
        zoomX.duration      = 500L
        zoomX.doOnEnd { splash.remove() }

        val zoomY = ObjectAnimator.ofFloat(
          splash.iconView,
          View.SCALE_Y,
          1.0f,
          0.0f
        )
        zoomY.interpolator  = OvershootInterpolator()
        zoomY.duration      = 500L
        zoomY.doOnEnd { splash.remove() }

        zoomX.start()
        zoomY.start()
      }
    }

    enableEdgeToEdge()
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

        val soundEffectState  = remember { SnackbarHostState() }
        val soundScope        = rememberCoroutineScope()
        ObserveAsEvents(FLOW = SoundController.effects)
        { event ->
          soundScope.launch ()
          {
            snackbarHostState.currentSnackbarData?.dismiss()
            SoundController.play(event)
          }
        }

        Scaffold(
          modifier      = Modifier.fillMaxSize(),
          snackbarHost  = { SnackbarHost(snackbarHostState) }
        )
        { innerPadding ->
          val navController = rememberNavController()
          val navigator     = App.appModule.navigator
          ObserveAsEvents(navigator.navigationAction)
          { action ->
            when(action)
            {
              is NavigationAction.Navigate          ->
                navController.navigate(action.DESTINATION)  {action.OPTIONS(this) }
              is NavigationAction.PopBackStack      ->
                {
                  if (action.DESTINATION != null) navController.popBackStack(action.DESTINATION, action.INCLUSIVE)
                  else                            navController.popBackStack()
                }
              is NavigationAction.ClearBackStack    ->
                {
                  navController.navigate(action.DESTINATION)
                  {
                    popUpTo(0) { inclusive = true}
                    launchSingleTop = true
                  }
                }
              is NavigationAction.NavigateSingleTop ->
                {
                  navController.navigate(action.DESTINATION) { launchSingleTop = true }
                }
              is NavigationAction.PopUpTo           ->
                {
                  navController.navigate(action.DESTINATION)
                  {
                    popUpTo(action.DESTINATION) { inclusive = action.INCLUSIVE}
                    launchSingleTop = true
                  }
                }
              is NavigationAction.Replace           ->
              {
                navController.popBackStack()
                navController.navigate(action.DESTINATION)
              }
              NavigationAction.NavigateBack         -> navController.navigateUp()
            }
          }

          NavHost(
            navController     = navController,
            startDestination  = navigator.startDestination,
            modifier          = Modifier.padding(innerPadding)
          )
          {
            composable<Destination.ScreenA>
            {
              ScreenA(
                VIEW_MODEL = viewModel<ScreenAViewModel>(factory = viewModelFactory()
                  { ScreenAViewModel(App.appModule.navigator) }),
              )
            }
            composable<Destination.ScreenB>
            {
              Box(
                modifier         = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
              )
              { Text("Screen B") }
            }
            composable<Destination.ArgScreen>
            {
              val args = it.toRoute<Destination.ArgScreen>()
              Text("ID: ${args.ID}")
            }
          }
        }
      }
    }
  }
}