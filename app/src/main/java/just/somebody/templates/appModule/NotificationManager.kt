package just.somebody.templates.appModule

import android.Manifest
import android.app.NotificationChannel
import android.content.Context
import android.os.Build
import androidx.annotation.DrawableRes
import androidx.annotation.RequiresPermission
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat

/**
 * Subsystem coordinator managing platform notification channels, status bar message dispatching,
 * and system interruption delivery configurations.
 */
interface NotificationManager
{
	/**
	 * Evaluates and orchestrates the initialization of a system-level notification pipeline.
	 * Required for target deployment environments running API level 26 and above.
	 *
	 * @param CONTEXT      Host execution reference path used to access system services.
	 * @param CHANNEL_ID   The targeted system pipeline identity path string.
	 * @param CHANNEL_NAME Human-readable nominal string presented to the user in OS settings.
	 * @param DESCRIPTION  Elaborative textual description for the channel context.
	 * @param IMPORTANCE   Integer flag defining visual and auditory interruption severity.
	 */
	fun createChannel(
		CONTEXT      : Context,
		CHANNEL_ID   : String,
		CHANNEL_NAME : String,
		DESCRIPTION  : String,
		IMPORTANCE   : Int)

	/**
	 * Dispatches an asynchronous user-facing interruption payload directly to the system status bar.
	 *
	 * @param CONTEXT         Host execution reference path used to access system services.
	 * @param CHANNEL_ID      The targeted system pipeline identity path string to route this payload.
	 * @param NOTIFICATION_ID Unique integer tracking identifier used for subsequent modifications.
	 * @param TITLE           Primary highlighted text string representing the event alert.
	 * @param MESSAGE         Secondary elaborative text string expanding on the alert context.
	 * @param ICON_RES        Platform drawable resource identifier bound to the visual display node.
	 */
	fun showNotification(
		CONTEXT         : Context,
		CHANNEL_ID      : String,
		NOTIFICATION_ID : Int,
		TITLE           : String,
		MESSAGE         : String,
		@DrawableRes
		ICON_RES        : Int,
		LARGE_ICON_URI  : String? = null)

	/**
	 * Cancels and flushes a previously dispatched asynchronous user-facing interruption payload.
	 *
	 * @param CONTEXT         Host execution reference path used to access system services.
	 * @param NOTIFICATION_ID Unique integer tracking identifier mapping to the active payload.
	 */
	fun cancelNotification(
		CONTEXT         : Context,
		NOTIFICATION_ID : Int)
}

/**
 * The standard notification manager
 */
class DefaultNotificationManager : NotificationManager
{
	override fun createChannel(
		CONTEXT      : Context,
		CHANNEL_ID   : String,
		CHANNEL_NAME : String,
		DESCRIPTION  : String,
		IMPORTANCE   : Int)
	{
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
		{
			val channel = NotificationChannel(CHANNEL_ID, CHANNEL_NAME, IMPORTANCE).apply()
			{
				description = DESCRIPTION
			}

			val notificationManager = CONTEXT.getSystemService(Context.NOTIFICATION_SERVICE) as android.app.NotificationManager
			notificationManager.createNotificationChannel(channel)
		}
	}

	@RequiresPermission(Manifest.permission.POST_NOTIFICATIONS)
	override fun showNotification(
		CONTEXT         : Context,
		CHANNEL_ID      : String,
		NOTIFICATION_ID : Int,
		TITLE           : String,
		MESSAGE         : String,
		@DrawableRes
		ICON_RES        : Int,
		LARGE_ICON_URI  : String?)
	{
		val builder = NotificationCompat.Builder(CONTEXT, CHANNEL_ID)
			.setSmallIcon(ICON_RES)
			.setContentTitle(TITLE)
			.setContentText(MESSAGE)
			.setPriority(NotificationCompat.PRIORITY_HIGH)
			.setAutoCancel(true)

		if (LARGE_ICON_URI != null) {
			try {
				val uri = android.net.Uri.parse(LARGE_ICON_URI)
				CONTEXT.contentResolver.openInputStream(uri)?.use { stream ->
					val bitmap = android.graphics.BitmapFactory.decodeStream(stream)
					if (bitmap != null) {
						builder.setLargeIcon(bitmap)
					}
				}
			} catch (e: Exception) {
				ForgeLogger.error("Failed to load large icon for notification: $e")
			}
		}

		val notificationManager = NotificationManagerCompat.from(CONTEXT)

		try
		{ notificationManager.notify(NOTIFICATION_ID, builder.build()) }
		catch (e: SecurityException)
		{ ForgeLogger.error(e); }
	}

	override fun cancelNotification(
		CONTEXT         : Context,
		NOTIFICATION_ID : Int)
	{
		val notificationManager = NotificationManagerCompat.from(CONTEXT)
		notificationManager.cancel(NOTIFICATION_ID)
	}
}