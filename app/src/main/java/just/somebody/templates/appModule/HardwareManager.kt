package just.somebody.templates.appModule

import android.Manifest
import android.content.Context
import android.net.ConnectivityManager
import android.net.ConnectivityManager.NetworkCallback
import android.net.Network
import android.net.NetworkCapabilities
import androidx.core.content.getSystemService
import just.somebody.templates.App
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow

/**
 * Closed structural hierarchy representing the granular lifecycle states of the system network link.
 */
sealed interface NetworkStatus
{
  /** Signal routing layer is fully established and authenticated for internet communication. */
  data object Available                       : NetworkStatus

  /** No operational hardware network interfaces or valid paths are currently available. */
  data object Unavailable                     : NetworkStatus

  /**
   * The active link is shedding packet priorities or preparing to drop.
   *
   * @property REMAINING_MS Approximate structural lifetime remaining on this network configuration.
   */
  data class  Losing(val REMAINING_MS : Int)  : NetworkStatus

  /** The primary active network connection has completely disconnected. */
  data object Lost                            : NetworkStatus
}

/**
 * Platform abstraction layer broker supervising continuous low-level hardware subsystem attributes.
 *
 * Exposes thread-safe reactive vectors to monitor system health and radio connectivity alterations.
 */
interface HardwareManager
{
  /** A continuous cold flow streaming real-time network capability adjustments from the OS. */
  val isConnectedToInternet : Flow<NetworkStatus>
}

class DefaultHardwareManager(
  private val CONTEXT : Context
                            ) : HardwareManager
{
  private val connectivityManager = CONTEXT.getSystemService<ConnectivityManager>()

  override val isConnectedToInternet : Flow<NetworkStatus>
    get() = callbackFlow()
    {
      val callback = object : NetworkCallback()
      {
        override fun onAvailable(NETWORK : Network)
        {
          super.onAvailable(NETWORK)
          trySend(NetworkStatus.Available)
        }

        override fun onUnavailable()
        {
          super.onUnavailable()
          trySend(NetworkStatus.Unavailable)
        }

        override fun onLosing(NETWORK : Network, MAX_MS_TO_LIVE : Int)
        {
          super.onLosing(NETWORK, MAX_MS_TO_LIVE)
          trySend(NetworkStatus.Losing(MAX_MS_TO_LIVE))
        }

        override fun onLost(network: Network)
        {
          super.onLost(network)
          trySend(NetworkStatus.Lost)
        }

        override fun onCapabilitiesChanged(
          NETWORK    : Network,
          CAPABILITY : NetworkCapabilities)
        {
          super.onCapabilitiesChanged(NETWORK, CAPABILITY)
          val connected = CAPABILITY.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)
          trySend(
            if (connected)  NetworkStatus.Available
            else            NetworkStatus.Unavailable)
        }
      }

      if (!App.appModule.permissionManager.hasPermission(CONTEXT, Manifest.permission.ACCESS_NETWORK_STATE) ||
          connectivityManager == null)
      { trySend(NetworkStatus.Unavailable) }
      else
      { connectivityManager.registerDefaultNetworkCallback(callback) }

      awaitClose { connectivityManager?.unregisterNetworkCallback(callback) }
    }
}