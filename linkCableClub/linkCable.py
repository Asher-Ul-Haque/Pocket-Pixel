import eventlet
from flask import Flask, request, render_template
from flask_socketio import SocketIO, emit, join_room
import uuid
import time

eventlet.monkey_patch()

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", transports=["websocket"])

sessions = {}
session_states = {}

TRANSFER_TIMEOUT = 2.0  # seconds


class TransferState:
    def __init__(self):
        self.sb_bytes = {}       # {sid: byte}
        self.sc_flags = {}       # {sid: sc}
        self.master_sid = None   # sid of current master
        self.last_transfer_time = 0.0


@app.route('/')
def index():
    return render_template("index.html")


@socketio.on('connect')
def on_connect():
    print(f"[CONNECT] Client connected: {request.sid}")


@socketio.on('disconnect')
def on_disconnect():
    print(f"[DISCONNECT] Client disconnected: {request.sid}")
    for sid, clients in list(sessions.items()):
        if request.sid in clients:
            clients.remove(request.sid)
            print(f"[LEAVE] {request.sid} removed from session {sid}")

            if sid in session_states:
                del session_states[sid]

            if not clients:
                del sessions[sid]
                print(f"[SESSION REMOVED] {sid}")
            elif len(clients) == 1:
                emit("partner_disconnected", room=clients[0])
                print(f"[NOTIFY] Partner in session {sid} notified of disconnect")


@socketio.on('create_session')
def create_session():
    session_id = str(uuid.uuid4())[:8]
    sessions[session_id] = [request.sid]
    session_states[session_id] = TransferState()
    join_room(session_id)
    print(f"[SESSION CREATED] {request.sid} created session {session_id}")
    emit('session_created', {'sessionId': session_id, 'status': 'Waiting for partner...'})


@socketio.on('join_session')
def join_session(data):
    session_id = data.get("sessionId")

    if session_id not in sessions:
        emit("session_error", {"message": "Session not found"})
        print(f"[ERROR] Join failed: Session {session_id} not found")
        return

    if len(sessions[session_id]) >= 2:
        emit("session_error", {"message": "Session is full"})
        print(f"[ERROR] Join failed: Session {session_id} is full")
        return

    sessions[session_id].append(request.sid)
    join_room(session_id)
    print(f"[JOINED] {request.sid} joined session {session_id}")
    emit("session_joined", {"sessionId": session_id, "status": "Connected!"})
    emit("partner_connected", {"sessionId": session_id}, room=session_id)


@socketio.on('send_link_data')
def send_link_data(data):
    session_id = data.get("sessionId")
    byte = data.get("byte")
    sc = data.get("sc", 0)
    current_time = time.time()

    if session_id not in sessions:
        print(f"[ERROR] Invalid session ID: {session_id}")
        return

    clients = sessions[session_id]
    if len(clients) != 2:
        print(f"[WAIT] Only one client in session {session_id}, skipping")
        return

    state = session_states[session_id]
    sender = request.sid
    recipient = [sid for sid in clients if sid != sender][0]

    # Store SB and SC
    state.sb_bytes[sender] = byte
    state.sc_flags[sender] = sc
    state.last_transfer_time = current_time

    # Detect master
    if (sc & 0x80) != 0:
        if state.master_sid is None:
            state.master_sid = sender
            print(f"[MASTER SET] {sender} is master for session {session_id}")
        elif state.master_sid != sender:
            print(f"[RACE] Both clients tried to be master in {session_id}")
            emit("error", {"message": "Both devices attempted to be master — transfer aborted"})
            return

    # Check if both clients are ready
    if len(state.sb_bytes) == 2 and state.master_sid is not None:
        master = state.master_sid
        slave = [sid for sid in clients if sid != master][0]

        master_byte = state.sb_bytes[master]
        slave_byte = state.sb_bytes[slave]

        # Full-duplex exchange
        emit("receive_link_data", {"byte": slave_byte}, room=master)
        emit("receive_link_data", {"byte": master_byte}, room=slave)

        print(f"[TRANSFER] {master} ⇄ {slave} | Bytes: {master_byte:02X} ⇄ {slave_byte:02X}")

        # Reset state
        state.sb_bytes.clear()
        state.sc_flags.clear()
        state.master_sid = None

    # Timeout handling
    elif current_time - state.last_transfer_time > TRANSFER_TIMEOUT:
        print(f"[TIMEOUT] Transfer timeout in session {session_id}, resetting state")
        state.sb_bytes.clear()
        state.sc_flags.clear()
        state.master_sid = None


if __name__ == '__main__':
    print("[STARTING] Link Cable Server running on http://0.0.0.0:5000")
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
