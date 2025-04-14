package org.just_somebody.pocket_pixel.splashScreen.presentation

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import org.just_somebody.pocket_pixel.core.Gamer
import org.just_somebody.pocket_pixel.core.onError
import org.just_somebody.pocket_pixel.core.onSuccess
import org.just_somebody.pocket_pixel.depInj.getGamer
import org.just_somebody.pocket_pixel.depInj.getNetworkCalls
import org.just_somebody.pocket_pixel.depInj.setGamer
import org.just_somebody.pocket_pixel.depInj.setGamerPass
import org.just_somebody.pocket_pixel.depInj.setGamerTag
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
      is SplashActions.ChangeName -> setGamerTag(ACTION.NAME)
      is SplashActions.ChangePass -> setGamerPass(ACTION.PASS)
      SplashActions.Login         -> login()
      SplashActions.Register      -> register()
      SplashActions.AutoLogin     -> autoLogin()
    }
  }

  private fun autoLogin()
  {
    viewModelScope.launch ()
    {
      state = state.copy(isLoggedIn  = false);
      setGamer(getGamerSessionStorage().getGamer() ?: getGamer());
      println(getGamer())
      login()
      state = state.copy(isLoginError = false)
    }
  }

  private fun login()
  {
    viewModelScope.launch ()
    {
      state       = state.copy(isLoggingIn = true);
      val result  = getNetworkCalls().loginGamer(getGamer())
      result.onSuccess ()
        {
          getGamerSessionStorage().saveGamer(getGamer())
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
      val result  = getNetworkCalls().registerGamer(getGamer())
      result.onSuccess ()
      {
        getGamerSessionStorage().saveGamer(getGamer())
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