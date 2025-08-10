package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.presentation.effects.SnackbarController
import just.somebody.templates.presentation.effects.SnackbarEvent
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch


class LinkCableViewModel : ViewModel()
{
  private val _isNetworkAvailable     : MutableStateFlow<NetworkStatus> = MutableStateFlow(NetworkStatus.Unavailable)
  public  val isNetworkAvailable      : StateFlow<NetworkStatus>        = _isNetworkAvailable

  private val _sessionID              : MutableStateFlow<String?>       = MutableStateFlow(null)
  public  val sessionID               : StateFlow<String?>              = _sessionID

  private val _isConnectedToServer    : MutableStateFlow<Boolean>       = MutableStateFlow(false)
  public  val isConnectedToServer     : StateFlow<Boolean>              = _isConnectedToServer

  private val _isPartnerConnected     : MutableStateFlow<Boolean>       = MutableStateFlow(false)
  public  val isPartnerConnected      : StateFlow<Boolean>              = _isPartnerConnected

  private val _isSessionNotFound      : MutableStateFlow<Boolean>       = MutableStateFlow(false)
  public  val isSessionNotFound       : StateFlow<Boolean>              = _isSessionNotFound

  private val _isSessionFull          : MutableStateFlow<Boolean>       = MutableStateFlow(false)
  public  val isSessionFull           : StateFlow<Boolean>              = _isSessionFull

  private val _waitingForTransfer     : MutableStateFlow<Boolean>       = MutableStateFlow(false)
  public  val waitingForTransfer      : StateFlow<Boolean>              = _waitingForTransfer

  val isInSession : StateFlow<Boolean> = _sessionID
    .map { !it.isNullOrBlank() }
    .stateIn(viewModelScope, SharingStarted.Eagerly, false)

  private val linkClient = App.appModule.linkCable

  init
  {
    observeInternetConnectivity()

    linkClient.connect()

    linkClient.onWaitingForPartner =
      { id ->
        _sessionID.value          = id
        _isPartnerConnected.value = false
        _isSessionNotFound.value  = false
        _isSessionFull.value      = false
        ForgeLogger.warn("Waiting for partner in session: $id")
      }

    linkClient.onSessionReady =
      {
        _isPartnerConnected.value = true
        ForgeLogger.warn("Partner connected")
      }

    linkClient.onPartnerDisconnected =
      {
        _isPartnerConnected.value = false
        _sessionID.value          = null
        ForgeLogger.warn("Partner disconnected")
      }

    linkClient.onSessionNotFound =
      {
        _isSessionNotFound.value  = true
        _isSessionFull.value      = false
        _sessionID.value          = null
        viewModelScope.launch { SnackbarController.sendEvent(SnackbarEvent("Session not found. Please check the ID.", null)) }
      }

    linkClient.onSessionFull =
      {
        _isSessionFull.value      = true
        _isSessionNotFound.value  = false
        _sessionID.value          = null
        viewModelScope.launch { SnackbarController.sendEvent(SnackbarEvent("Session is full. Try another ID.", null)) }
      }

    linkClient.onWaitingForTransferPartner =
      {
        _waitingForTransfer.value = true
      }

    linkClient.onByteReceived =
      {
        _waitingForTransfer.value = false
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
    _sessionID.value          = null
    _isPartnerConnected.value = false
    _isSessionNotFound.value  = false
    _isSessionFull.value      = false
    _waitingForTransfer.value = false
  }

  fun createSession()
  { linkClient.joinSession(null) }

  fun joinSession(ID : String)
  { linkClient.joinSession(ID) }

  fun disconnect()
  {
    resetSession()
    linkClient.disconnect()
    _isConnectedToServer.value = false
  }
}