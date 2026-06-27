package just.somebody.templates

import android.app.Application
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.annotation.RequiresApi
import just.somebody.templates.appModule.AppModule
import just.somebody.templates.appModule.AppModuleInterface

class App() : Application()
{
  companion object { lateinit var appModule : AppModuleInterface }

  override fun onCreate()
  {
    super.onCreate()
    appModule = AppModule(this)

    appModule.notificationManager.createChannel(
      this,
      "ACHIEVEMENTS",
      "Achievements",
      "Notifications for unlocked achievements",
      NotificationManager.IMPORTANCE_HIGH
    )
  }
}