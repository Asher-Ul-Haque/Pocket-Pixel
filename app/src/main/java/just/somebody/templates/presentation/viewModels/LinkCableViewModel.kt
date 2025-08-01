package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch

class LinkCableViewModel : ViewModel()
{
  private val _isNetworkAvailable : MutableStateFlow<NetworkStatus> = MutableStateFlow(NetworkStatus.Unavailable)
  public  val isNetworkAvailable  : StateFlow<NetworkStatus>        = _isNetworkAvailable

  private val _sessionID : MutableStateFlow<String?>  = MutableStateFlow(null)
  public  val sessionID  : StateFlow<String?>         = _sessionID

  private val _isConnectedToServer : MutableStateFlow<Boolean>  = MutableStateFlow(false)
  public  val isConnectedToServer  : StateFlow<Boolean>         = _isConnectedToServer

  private val _isPartnerConnected : MutableStateFlow<Boolean> = MutableStateFlow(false)
  public  val isPartnerConnected  : StateFlow<Boolean>        = _isPartnerConnected

  val isInSession : StateFlow<Boolean> = _sessionID
    .map { !it.isNullOrBlank() }
    .stateIn(viewModelScope, SharingStarted.Eagerly, false)

  private val linkClient = App.appModule.linkCable

  init
  {
    observeInternetConnectivity()

    linkClient.connect()

    linkClient.onSessionCreated =
      { id ->
        _sessionID.value = id
        ForgeLogger.warn("Created session with id: $id")
      }

    linkClient.onSessionJoined =
      { id ->
        _sessionID.value = id
        ForgeLogger.warn("Joined session with id: $id")
      }

    linkClient.onPartnerConnected =
      {
        _isPartnerConnected.value = true
        ForgeLogger.warn("Partner connected")
      }

    linkClient.onPartnerDisconnected =
      {
        _isPartnerConnected.value = false
        ForgeLogger.warn("Partner disconnected")
      }

    linkClient.onError =
      { _isConnectedToServer.value = false }

    viewModelScope.launch()
    {
      while (true)
      {
        if (_isNetworkAvailable.value == NetworkStatus.Available)
        {
          if (_isConnectedToServer.value && !linkClient.connected)
          { linkClient.connect() }
          _isConnectedToServer.value = linkClient.connected
        }

        kotlinx.coroutines.delay(1000)
      }
    }
  }

  private fun observeInternetConnectivity()
  {
    viewModelScope.launch()
    {
      App.appModule.hardwareManager.isConnectedToInternet
        .distinctUntilChanged()
        .collect { status -> _isNetworkAvailable.value = status }
    }
  }

  fun resetSession()
  {
    _sessionID.value = null
    _isPartnerConnected.value = false
  }

  fun createSession()
  { linkClient.createSession() }

  fun joinSession(ID : String)
  { linkClient.joinSession(ID) }

  fun disconnect()
  {
    resetSession()
    _isConnectedToServer.value = false
  }
}
