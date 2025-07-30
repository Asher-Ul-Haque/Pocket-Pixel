package just.somebody.templates.presentation.viewModels

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.distinctUntilChanged
import kotlinx.coroutines.launch

class LinkCableViewModel : ViewModel()
{
  private val _isConnected : MutableStateFlow<Boolean> = MutableStateFlow<Boolean>(false)
  public  val isConnected  : StateFlow<Boolean>        = _isConnected

  private val _isPartnerConnected : MutableStateFlow<Boolean> = MutableStateFlow<Boolean>(false)
  public  val isPartnerConnected  : StateFlow<Boolean>        = _isPartnerConnected

  private val _sessionID : MutableStateFlow<String?> = MutableStateFlow<String?>(null)
  public  val sessionID  : StateFlow<String?>        = _sessionID

  private fun observeInternetConnectivity()
  {
    viewModelScope.launch ()
    {
      App.appModule.hardwareManager.isConnectedToInternet
        .distinctUntilChanged()
        .collect ()
        { status ->
          if (status is NetworkStatus.Available)  _isConnected.value = true
          else                                    _isConnected.value = false
        }
    }
  }

  init
  {
    observeInternetConnectivity()

    val linkClient = App.appModule.linkCable
    linkClient.connect()
    linkClient.onSessionCreated      =
      {
        id -> _sessionID.value = id
        ForgeLogger.warn("Created sesisons with id : ${id}")
      }
    linkClient.onSessionJoined       = { id -> _sessionID.value = id }
    linkClient.onPartnerConnected    = { _isPartnerConnected.value = true }
    linkClient.onPartnerDisconnected = { _isPartnerConnected.value = false }
  }

  fun resetSession()
  {
    _sessionID.value          = null
    _isPartnerConnected.value = false
  }
}