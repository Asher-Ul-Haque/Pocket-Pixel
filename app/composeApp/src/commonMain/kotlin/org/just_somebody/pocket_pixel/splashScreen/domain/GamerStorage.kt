package org.just_somebody.pocket_pixel.splashScreen.domain

abstract class GamerSessionStorage
{
  abstract suspend fun getGamer   () : Gamer?
  abstract suspend fun saveGamer  (GAMER : Gamer)
  abstract suspend fun clearGamer ()
}

expect fun getGamerSessionStorage () : GamerSessionStorage;