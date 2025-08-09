import io.socket.client.IO
import io.socket.client.Socket
import just.somebody.templates.appModule.ForgeLogger
import org.json.JSONObject
import java.net.URISyntaxException


class LinkCable
{
  // - - - Socket Info
  private val baseUrl         = "https://pocket-pixel-link-cable-club.onrender.com/"
  private var socket: Socket? = null

  // - - - State
  var currentSessionId : String?  = null
    private set
  var connected        : Boolean  = false
    private set

  // - - - Callbacks
  var onSessionReady              : (() -> Unit)?       = null
  var onWaitingForPartner         : ((String) -> Unit)? = null
  var onPartnerDisconnected       : (() -> Unit)?       = null
  var onByteReceived              : ((Int) -> Unit)?    = null
  var onWaitingForTransferPartner : (() -> Unit)?       = null
  var onError                     : ((String) -> Unit)? = null
  var onSessionNotFound           : (() -> Unit)?       = null
  var onSessionFull               : (() -> Unit)?       = null

  // - - - Connect to server
  fun connect()
  {
    if (socket != null) return ForgeLogger.warn("Socket already initialized")

    try
    {
      val options = IO.Options().apply()
      {
        reconnection  = true
        forceNew      = true
        transports    = arrayOf("websocket")
      }

      socket = IO.socket(baseUrl, options).apply()
      {
        on(Socket.EVENT_CONNECT)
        {
          ForgeLogger.info("Connected to link cable server")
          connected = true
        }

        on(Socket.EVENT_DISCONNECT)
        {
          ForgeLogger.warn("Disconnected from link cable server")
          connected = false
        }

        on(Socket.EVENT_CONNECT_ERROR)
        { args ->
          val msg = "Connection error: ${args.joinToString()}"
          ForgeLogger.error(msg)
          onError?.invoke(msg)
        }

        on("waiting_for_partner")
        { args ->
          val sessionId = (args.getOrNull(0) as? JSONObject)?.optString("session_id")
          if (!sessionId.isNullOrBlank())
          {
            currentSessionId = sessionId
            ForgeLogger.info("Waiting for partner in session: $sessionId")
            onWaitingForPartner?.invoke(sessionId)
          }
        }

        on("session_ready")
        {
          ForgeLogger.info("Partner connected and session is ready")
          onSessionReady?.invoke()
        }

        on("session_not_found")
        {
          ForgeLogger.warn("Attempted to join a session that does not exist.")
          onSessionNotFound?.invoke()
        }

        on("session_full")
        {
          ForgeLogger.warn("Attempted to join a session that is already full.")
          onSessionFull?.invoke()
        }

        on("waiting_for_transfer_partner")
        {
          ForgeLogger.info("Transfer sent, waiting for partner to send their byte.")
          onWaitingForTransferPartner?.invoke()
        }

        on("partner_disconnected")
        {
          ForgeLogger.warn("Partner disconnected")
          onPartnerDisconnected?.invoke()
        }

        on("receive_link_data")
        { args ->
          val byte = (args.getOrNull(0) as? JSONObject)?.optInt("byte", -1) ?: -1
          if (byte in 0..255)
          {
            ForgeLogger.trace("Received byte: $byte")
            onByteReceived?.invoke(byte)
          }
        }
      }

      ForgeLogger.info("Connecting to $baseUrl")
      socket?.connect()
    }
    catch (e: URISyntaxException)
    {
      ForgeLogger.error("Invalid URI: ${e.message}")
      onError?.invoke(e.message ?: "Unknown URI syntax error")
    }
    catch (e: Exception)
    {
      ForgeLogger.error("Connection failed: ${e.message}")
      onError?.invoke(e.message ?: "Unknown socket error")
    }
  }

  // - - - Emit commands
  fun joinSession(SESSION_ID : String?)
  {
    ifNotConnected { return }

    val payload = if (SESSION_ID.isNullOrBlank())
    {
      ForgeLogger.info("Creating a new session.")
      JSONObject()
    }
    else
    {
      ForgeLogger.info("Joining session: $SESSION_ID")
      JSONObject().put("session_id", SESSION_ID)
    }

    socket?.emit("join_session", payload)
    currentSessionId = SESSION_ID
  }

  fun sendByte(SB : Int)
  {
    ifNotConnected { return }
    val payload = JSONObject()
      .put("byte",   SB)

    ForgeLogger.trace("Sending byte: $SB")
    socket?.emit("send_link_data", payload)
  }

  fun disconnect()
  {
    ForgeLogger.warn("Disconnecting from link cable")
    socket?.disconnect()
    socket?.off()
    socket            = null
    currentSessionId  = null
    connected         = false
  }

  // - - - Helpers
  private inline fun ifNotConnected(ACTION : () -> Unit)
  {
    if (!connected || socket == null || !socket!!.connected())
    {
      ForgeLogger.error("Socket not connected")
      ACTION()
    }
  }
}