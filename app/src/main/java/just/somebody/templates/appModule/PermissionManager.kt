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

/**
 * Subsystem coordinator managing platform validation rules, system verification queries,
 * and contextual redirection links for Android runtime permissions.
 */
interface PermissionManager
{
  /** Evaluates whether an explicit single security permission string is currently authorized. */
  fun hasPermission(CONTEXT : Context, PERMISSION: String): Boolean

  /**
   * Evaluates, orchestrates, and invokes a single runtime dialog approval prompt targeting a Composable layout node.
   *
   * @param PERMISSION The targeted system security identity path string.
   * @param ON_GRANT Callback triggered when user configuration verification passes.
   * @param TRIGGER Flag that fires the underlying asynchronous manifest authorization window.
   * @param ON_TRIGGER Callback tracking completion of permission check processing.
   * @param GO_TO_SETTINGS Directs the execution thread to open system properties if permanently denied.
   */
  @Composable
  fun RequestPermissionIfNeeded(
    PERMISSION      : String,
    ON_GRANT        : () -> Unit,
    TRIGGER         : Boolean,
    ON_TRIGGER      : () -> Unit,
    GO_TO_SETTINGS  : Boolean)

  /**
   * Evaluates, orchestrates, and invokes a multi-permission verification matrix block inside a Composable layout node.
   *
   * @param PERMISSIONS Matched array list of required authorization properties.
   * @param ON_ALL_GRANT Callback triggered when all combined parameters successfully map to active approvals.
   * @param TRIGGER Flag that fires the underlying asynchronous manifest authorization window.
   * @param ON_TRIGGER Callback tracking completion of permission check processing.
   * @param GO_TO_SETTINGS Directs the execution thread to open system properties if permanently denied.
   */
  @Composable
  fun RequestPermissionIfNeeded(
    PERMISSIONS    : Array<String>,
    ON_ALL_GRANT   : () -> Unit,
    TRIGGER        : Boolean,
    ON_TRIGGER     : () -> Unit,
    GO_TO_SETTINGS : Boolean)
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
    GO_TO_SETTINGS  : Boolean)
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
    GO_TO_SETTINGS : Boolean)
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
    /**
     * Issues an implicit platform intent vector tracking application identifier paths to show OS settings info directly.
     *
     * @param context Host execution reference path used to initiate the target transaction task.
     */
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