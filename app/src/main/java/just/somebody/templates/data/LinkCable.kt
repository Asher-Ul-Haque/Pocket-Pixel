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
  var sentByte     : Int = 0
  var receivedByte : Int = 0

  // - - - Callbacks
  var onSessionCreated      : ((String) -> Unit)? = null
  var onSessionJoined       : ((String) -> Unit)? = null
  var onPartnerConnected    : (() -> Unit)?       = null
  var onPartnerDisconnected : (() -> Unit)?       = null
  var onByteReceived        : ((Int) -> Unit)?    = null
  var onError               : ((String) -> Unit)? = null

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

        on("session_created")
        { args ->
          val sessionId = (args.getOrNull(0) as? JSONObject)?.optString("sessionId")
          if (!sessionId.isNullOrBlank())
          {
            currentSessionId = sessionId
            ForgeLogger.info("Session created: $sessionId")
            onSessionCreated?.invoke(sessionId)
          }
        }

        on("session_joined")
        { args ->
          val sessionId = (args.getOrNull(0) as? JSONObject)?.optString("sessionId")
          if (!sessionId.isNullOrBlank())
          {
            currentSessionId = sessionId
            ForgeLogger.info("Session joined: $sessionId")
            onSessionJoined?.invoke(sessionId)
          }
        }

        on("partner_connected")
        {
          ForgeLogger.info("Partner connected")
          onPartnerConnected?.invoke()
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
            receivedByte = byte
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
  fun createSession()
  {
    ifNotConnected { return }
    socket?.emit("create_session")
  }

  fun joinSession(SESSION_ID : String)
  {
    ifNotConnected { return }
    val payload = JSONObject().put("sessionId", SESSION_ID)
    socket?.emit("join_session", payload)
    currentSessionId = SESSION_ID
  }

  fun sendByte(BYTE : Int)
  {
    ifNotConnected { return }
    val sessionId = currentSessionId ?: return ForgeLogger.error("No session to send byte")

    val payload = JSONObject()
      .put("sessionId", sessionId)
      .put("byte", BYTE)

    ForgeLogger.trace("Sending byte: $BYTE to $sessionId")
    sentByte = BYTE
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
