import io.socket.client.IO
import io.socket.client.Socket
import just.somebody.templates.appModule.ForgeLogger
import org.json.JSONObject
import java.net.URISyntaxException

class LinkCable {
  private val baseUrl       = "http://192.168.1.11:5000/"
  private var socket        : Socket? = null
  var onSessionCreated      : ((String) -> Unit)? = null
  var onSessionJoined       : ((String) -> Unit)? = null
  var onPartnerConnected    : (() -> Unit)? = null
  var onPartnerDisconnected : (() -> Unit)? = null
  var onByteReceived        : ((Int) -> Unit)? = null
  var friendSessionID       : String = ""

  fun connect() {
    try {
      val options = IO.Options().apply {
        reconnection = true
        forceNew = true
        transports = arrayOf("websocket") // <-- force WebSocket only
      }


      socket = IO.socket(baseUrl, options)

      socket?.on(Socket.EVENT_CONNECT) {
        ForgeLogger.info("Socket EVENT_CONNECT")
      }

      socket?.on(Socket.EVENT_CONNECT_ERROR) { args ->
        ForgeLogger.error("Socket EVENT_CONNECT_ERROR: ${args.joinToString()}")
      }

      socket?.on(Socket.EVENT_DISCONNECT) {
        ForgeLogger.warn("Socket EVENT_DISCONNECT")
      }

      socket?.on("session_created") { args ->
        ForgeLogger.info("Received 'session_created' event")
        args.forEachIndexed { i, arg ->
          ForgeLogger.trace("Arg[$i]: $arg")
        }

        try {
          val data = args[0] as JSONObject
          val sessionId = data.getString("sessionId")
          ForgeLogger.info("Session created with ID: $sessionId")
          onSessionCreated?.invoke(sessionId)
        } catch (e: Exception) {
          ForgeLogger.error("Failed to parse session_created payload: ${e.message}")
        }
      }

      socket?.on("session_joined") { args ->
        ForgeLogger.info("Received 'session_joined' event")
        args.forEachIndexed { i, arg ->
          ForgeLogger.trace("Arg[$i]: $arg")
        }

        try {
          val data = args[0] as JSONObject
          val sessionId = data.getString("sessionId")
          ForgeLogger.info("Session joined with ID: $sessionId")
          onSessionJoined?.invoke(sessionId)
        } catch (e: Exception) {
          ForgeLogger.error("Failed to parse session_joined payload: ${e.message}")
        }
      }

      socket?.on("partner_connected") {
        ForgeLogger.info("Received 'partner_connected'")
        onPartnerConnected?.invoke()
      }

      socket?.on("partner_disconnected") {
        ForgeLogger.info("Received 'partner_disconnected'")
        onPartnerDisconnected?.invoke()
      }

      socket?.on("receive_link_data") { args ->
        ForgeLogger.info("Received 'receive_link_data'")
        args.forEachIndexed { i, arg ->
          ForgeLogger.trace("Arg[$i]: $arg")
        }

        try {
          val data = args[0] as JSONObject
          val byte = data.getInt("byte")
          ForgeLogger.info("Received byte: $byte")
          onByteReceived?.invoke(byte)
        } catch (e: Exception) {
          ForgeLogger.error("Failed to parse byte: ${e.message}")
        }
      }


      ForgeLogger.info("Connecting to Socket.IO server: $baseUrl")
      socket?.connect()
    } catch (e: URISyntaxException) {
      ForgeLogger.error("Socket URI error: ${e.message}")
    } catch (e: Exception) {
      ForgeLogger.error("Socket connection error: ${e.message}")
    }
  }

  fun createSession() {
    ForgeLogger.info("Emitting 'create_session'")
    ForgeLogger.trace("Is socket null: ${socket == null}")
    socket?.emit("create_session")
  }

  fun joinSession(sessionId: String) {
    ForgeLogger.info("Emitting 'join_session' with ID: $sessionId")
    val payload = JSONObject().put("sessionId", sessionId)
    socket?.emit("join_session", payload)
    friendSessionID = sessionId
  }

  fun sendByte(byte: Int) {
    ForgeLogger.info("Emitting 'send_link_data': $byte")
    val payload = JSONObject()
      .put("sessionId", friendSessionID)
      .put("byte", byte)
    socket?.emit("send_link_data", payload)
  }

  fun disconnect() {
    ForgeLogger.warn("Disconnecting socket")
    socket?.disconnect()
    socket?.off()
    socket = null
  }
}
