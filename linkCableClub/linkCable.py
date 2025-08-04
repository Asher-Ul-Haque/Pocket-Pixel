import eventlet
from flask          import Flask, render_template, request
from flask_socketio import SocketIO, emit, join_room, close_room
import uuid
import time
eventlet.monkey_patch()


app      = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", transports=["websocket"])

# Dictionary to store active sessions.
# Key: session_id, Value: set of connected client sids
sessions = {}

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def on_connect():
    print(f"[CONNECT] Client connected: {request.sid}")

@socketio.on('disconnect')
def on_disconnect():
    print(f"[DISCONNECT] Client disconnected: {request.sid}")
    # Find which session this client was part of
    for session_id, clients_in_session in list(sessions.items()):
        if request.sid in clients_in_session:
            clients_in_session.remove(request.sid)
            print(f"[LEAVE] {request.sid} removed from session {session_id}")

            # Notify remaining client if there's only one left
            if len(clients_in_session) == 1:
                remaining_client_sid = list(clients_in_session)[0]
                emit("partner_disconnected", room=remaining_client_sid)
                print(f"[NOTIFY] Partner disconnected in session {session_id}, notifying {remaining_client_sid}")
            elif not clients_in_session:
                # If no clients left, remove the session
                del sessions[session_id]
                close_room(session_id) # Ensure the room is truly closed
                print(f"[SESSION REMOVED] {session_id}")
            break # Exit loop once the client's session is found and processed

@socketio.on('join_session')
def on_join_session(data):
    session_id = data.get('session_id')
    if not session_id:
        # Generate a new session ID if none provided (for the first client)
        session_id = str(uuid.uuid4())
        print(f"[CREATE] New session created: {session_id}")

    if session_id not in sessions:
        sessions[session_id] = set()

    clients_in_session = sessions[session_id]

    if len(clients_in_session) < 2:
        join_room(session_id)
        clients_in_session.add(request.sid)
        print(f"[JOIN] {request.sid} joined session {session_id}. Current clients: {len(clients_in_session)}")

        if len(clients_in_session) == 2:
            print(f"[READY] Session {session_id} is full. Both clients connected.")
            # Notify both clients that the session is ready
            emit('session_ready', room=session_id)
        else:
            emit('waiting_for_partner', {'session_id': session_id}, room=request.sid)
            print(f"[WAIT] {request.sid} waiting for partner in session {session_id}.")
    else:
        print(f"[REJECT] Session {session_id} is full. {request.sid} rejected.")
        emit('session_full', room=request.sid)


@socketio.on('send_link_data')
def on_send_link_data(data):
    byte = data.get('byte')
    if byte is None:
        print(f"[ERROR] Received send_link_data without 'byte' from {request.sid}")
        return

    # Find the session this client belongs to
    session_id = None
    for sid, clients_in_session in sessions.items():
        if request.sid in clients_in_session:
            session_id = sid
            break

    if not session_id:
        print(f"[ERROR] {request.sid} not in any active session, ignoring data.")
        return

    clients_in_session = sessions[session_id]
    if len(clients_in_session) != 2:
        print(f"[WARN] Data from {request.sid} in incomplete session {session_id}. Clients: {len(clients_in_session)}")
        return

    # Find the recipient (the other client in the same session)
    recipient_sid = None
    for client_sid in clients_in_session:
        if client_sid != request.sid:
            recipient_sid = client_sid
            break

    if recipient_sid:
        # Forward the byte directly to the other client
        print(f"[RELAY] {request.sid} -> {recipient_sid}, byte: 0x{byte:02X}")
        emit('receive_link_data', {'byte': byte}, room=recipient_sid)
    else:
        print(f"[WARN] No recipient found for {request.sid} in session {session_id}.")

if __name__ == '__main__':
    print("Starting Flask-SocketIO server for Game Boy Link Cable emulation...")
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)

