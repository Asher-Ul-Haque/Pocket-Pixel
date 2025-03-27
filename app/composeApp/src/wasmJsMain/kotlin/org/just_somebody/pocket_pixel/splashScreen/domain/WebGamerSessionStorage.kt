package org.just_somebody.pocket_pixel.splashScreen.domain

import kotlinx.browser.window
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json

class WebGamerSessionStorage : GamerSessionStorage()
{
  private val storageKey = "lastLoggedInUser"

  override suspend fun getGamer   (): Gamer?
  {
    val gamerJson = window.localStorage.getItem(storageKey) ?: return null
    return Json.decodeFromString<Gamer>(gamerJson)
  }

  override suspend fun saveGamer (GAMER : Gamer)
  {
    window.localStorage.setItem(storageKey, Json.encodeToString(GAMER))
  }

  override suspend fun clearGamer ()
  {
    window.localStorage.removeItem(storageKey)
  }
}

actual fun getGamerSessionStorage(): GamerSessionStorage
{
  return WebGamerSessionStorage();
}