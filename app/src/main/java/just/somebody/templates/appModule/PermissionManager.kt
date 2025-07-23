package just.somebody.templates.appModule

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.provider.Settings
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.*
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.compose.ui.platform.LocalContext


interface PermissionManager
{
  fun hasPermission(CONTEXT : Context, PERMISSION: String): Boolean

  @Composable
  fun RequestPermissionIfNeeded(
    PERMISSION      : String,
    ON_GRANT        : () -> Unit,
    TRIGGER         : Boolean,
    ON_TRIGGER      : () -> Unit,
    GO_TO_SETTINGS  : Boolean
  )

  @Composable
  fun RequestPermissionIfNeeded(
    PERMISSIONS    : Array<String>,
    ON_ALL_GRANT   : () -> Unit,
    TRIGGER        : Boolean,
    ON_TRIGGER     : () -> Unit,
    GO_TO_SETTINGS : Boolean
  )
}

class DefaultPermissionManager : PermissionManager
{
  override fun hasPermission(CONTEXT: Context, PERMISSION: String): Boolean
  { return ContextCompat.checkSelfPermission(CONTEXT, PERMISSION) == PackageManager.PERMISSION_GRANTED }

  @Composable
  override fun RequestPermissionIfNeeded(
    PERMISSION      : String,
    ON_GRANT        : () -> Unit,
    TRIGGER         : Boolean,
    ON_TRIGGER      : () -> Unit,
    GO_TO_SETTINGS  : Boolean
  )
  {
    val context       = LocalContext.current
    val activity      = context as? Activity
    var launcherReady by remember { mutableStateOf(false) }
    val launcher      = rememberLauncherForActivityResult(ActivityResultContracts.RequestPermission())
    { isGranted ->
      if (isGranted) ON_GRANT()
      else if (
        activity != null                                                           &&
        !ActivityCompat.shouldShowRequestPermissionRationale(activity, PERMISSION) &&
        !hasPermission(context, PERMISSION)                                        &&
        GO_TO_SETTINGS
      ) openAppSettings(context)

      ON_TRIGGER()
    }

    LaunchedEffect(Unit) { launcherReady = true }

    LaunchedEffect(TRIGGER, launcherReady)
    { if (TRIGGER && launcherReady) launcher.launch(PERMISSION) }
  }

  @Composable
  override fun RequestPermissionIfNeeded(
    PERMISSIONS    : Array<String>,
    ON_ALL_GRANT   : () -> Unit,
    TRIGGER        : Boolean,
    ON_TRIGGER     : () -> Unit,
    GO_TO_SETTINGS : Boolean
  )
  {
    val context       = LocalContext.current
    val activity      = context as? Activity
    var launcherReady by remember { mutableStateOf(false) }

    val launcher = rememberLauncherForActivityResult(ActivityResultContracts.RequestMultiplePermissions())
    { resultMap ->
      val allGranted = resultMap.values.all { it }
      if (allGranted) ON_ALL_GRANT()
      else if (activity != null && GO_TO_SETTINGS)
      {
        val permanentlyDeclined = PERMISSIONS.any()
        { permission ->
          !ActivityCompat.shouldShowRequestPermissionRationale(activity, permission) &&
          !hasPermission(context, permission)
        }
        if (permanentlyDeclined) openAppSettings(context)
      }

      ON_TRIGGER()
    }

    LaunchedEffect(Unit) { launcherReady = true }

    LaunchedEffect(TRIGGER, launcherReady)
    { if (TRIGGER && launcherReady) { launcher.launch(PERMISSIONS) } }
  }

  companion object
  {
    fun openAppSettings(context: Context)
    {
      val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply()
      {
        data = Uri.fromParts("package", context.packageName, null)
        addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
      }
      context.startActivity(intent)
    }
  }
}
