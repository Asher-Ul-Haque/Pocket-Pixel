package org.just_somebody.pocket_pixel.splashScreen.presentation

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.networking.createHttpClient
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getSplashNetworkCalls
import org.just_somebody.pocket_pixel.splashScreen.domain.Gamer
import org.just_somebody.pocket_pixel.splashScreen.domain.getGamerSessionStorage

class SplashViewModel : ViewModel()
{
  var state by mutableStateOf(SplashState())
    private set;

  fun onAction(ACTION : SplashActions)
  {
    println("doing action : $ACTION");
    when (ACTION)
    {
      is SplashActions.ChangeName ->
        state = state.copy(gamer = Gamer(ACTION.NAME, state.gamer.password));

      is SplashActions.ChangePass ->
        state = state.copy(gamer = Gamer(state.gamer.name, ACTION.PASS));

      SplashActions.Login         ->
        login()

      SplashActions.Register      ->
        register()

      SplashActions.AutoLogin     ->
        autoLogin()
    }
  }

  private fun autoLogin()
  {
    viewModelScope.launch ()
    {
      state = state.copy(
        isLoggedIn  = false,
        gamer       = getGamerSessionStorage().getGamer() ?: state.gamer);
      println(state.gamer)
      login()
      state = state.copy(isLoginError = false)
    }
  }

  private fun login()
  {
    viewModelScope.launch ()
    {
      state       = state.copy(isLoggingIn = true);
      val result  = getSplashNetworkCalls().connectToServer(state.gamer)
      result.onSuccess ()
        {
          getGamerSessionStorage().saveGamer(state.gamer)
          println("Wow, we did it");
          state = state.copy(
            isLoggedIn    = true,
            isLoggingIn   = false,
            isLoginError  = false
          )
        }
      result.onError ()
        {
          getGamerSessionStorage().clearGamer()
          println("we failed it");
          state = state.copy(
            isLoggedIn    = false,
            isLoggingIn   = false,
            isLoginError  = true)
        }
    }
  }

  private fun register()
  {
    viewModelScope.launch ()
    {
      state       = state.copy(isLoggingIn = true);
      val result  = getSplashNetworkCalls().registerGamer(state.gamer)
      result.onSuccess ()
      {
        getGamerSessionStorage().saveGamer(state.gamer)
        println("Wow, we did it");
        state = state.copy(
          isLoggedIn    = true,
          isLoggingIn   = false,
          isLoginError  = false
        )
        login()
      }
      result.onError ()
      {
        getGamerSessionStorage().clearGamer()
        println("we failed it");
        state = state.copy(
          isLoggedIn    = false,
          isLoggingIn   = false,
          isLoginError  = true)
      }
    }
  }
}