package org.just_somebody.pocket_pixel.splashScreen.domain

import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import android.content.Context
import org.just_somebody.pocket_pixel.splashScreen.depInj.getContext

class AndroidGamerSessionStorage() : GamerSessionStorage()
{
  private val storageKey = "lastLoggedInUser"

  override suspend fun getGamer   (): Gamer?
  {
    val sharedPreferences = getContext().getSharedPreferences(storageKey, Context.MODE_PRIVATE);
    val gamerJson         = sharedPreferences.getString(storageKey, null);
    return gamerJson?.let { Json.decodeFromString<Gamer>(it); };
  }

  override suspend fun saveGamer (GAMER : Gamer)
  {
    val sharedPreferences = getContext().getSharedPreferences(storageKey, Context.MODE_PRIVATE);
    val editor            = sharedPreferences.edit();
    editor.putString(storageKey, Json.encodeToString(GAMER));
    editor.apply();
  }

  override suspend fun clearGamer ()
  {
    val sharedPreferences = getContext().getSharedPreferences(storageKey, Context.MODE_PRIVATE);
    val editor            = sharedPreferences.edit();
    editor.remove(storageKey);
    editor.apply();
  }
}

actual fun getGamerSessionStorage(): GamerSessionStorage
{
  return AndroidGamerSessionStorage()
}