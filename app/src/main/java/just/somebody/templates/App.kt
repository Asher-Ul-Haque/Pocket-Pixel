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
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
    {
      val notificationChannel = NotificationChannel(
        "channel_id",
        "Notifications",
        NotificationManager.IMPORTANCE_DEFAULT
      )
      notificationChannel.description = "Notifications for this app"
      val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
      notificationManager.createNotificationChannel(notificationChannel)
    }
  }
}