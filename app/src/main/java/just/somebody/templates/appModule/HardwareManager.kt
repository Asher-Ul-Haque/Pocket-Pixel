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

sealed interface NetworkStatus
{
  data object Available                       : NetworkStatus
  data object Unavailable                     : NetworkStatus
  data class  Losing(val REMAINING_MS : Int)  : NetworkStatus
  data object Lost                            : NetworkStatus
}
interface HardwareManager
{
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
              else            NetworkStatus.Unavailable
            )
          }
        }

        if (!App.appModule.permissionManager.hasPermission(CONTEXT, Manifest.permission.ACCESS_NETWORK_STATE) ||
          connectivityManager == null) trySend(NetworkStatus.Unavailable)
        else connectivityManager.registerDefaultNetworkCallback(callback)

        awaitClose { connectivityManager?.unregisterNetworkCallback(callback) }
      }
}