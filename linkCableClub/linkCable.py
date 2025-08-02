import eventlet
from flask          import Flask, render_template, request
from flask_socketio import SocketIO, emit, join_room
import uuid
import time
eventlet.monkey_patch()


app      = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", transports=["websocket"])

# - - - Enhanced session tracking
sessions        = {}
session_states  = {}  # - - - Track transfer states per session

class TransferState:
    def __init__(self):
        self.waiting_for_response   = False
        self.master_client          = None
        self.master_byte            = None
        self.last_transfer_time     = 0

@app.route('/')
def index(): return render_template('index.html')

@socketio.on('connect')
def on_connect(): print(f"[CONNECT] Client connected: {request.sid}")

@socketio.on('disconnect')
def on_disconnect():
    print(f"[DISCONNECT] Client disconnected: {request.sid}")
    for sid, clients in list(sessions.items()):
        if request.sid in clients:
            clients.remove(request.sid)
            print(f"[LEAVE] {request.sid} removed from session {sid}")

            # - - - Clean up session state
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
    session_id                  = str(uuid.uuid4())[:8]
    sessions[session_id]        = [request.sid]
    session_states[session_id]  = TransferState()
    join_room(session_id)
    print(f"[SESSION CREATED] {request.sid} created session {session_id}")
    emit('session_created', {'sessionId': session_id, 'status': 'Waiting for partner...'})

@socketio.on('join_session')
def join_session(data):
    session_id = data.get('sessionId')
    print(f"[JOIN ATTEMPT] {request.sid} attempting to join session {session_id}")

    if session_id not in sessions:
        print(f"[ERROR] Session {session_id} not found")
        emit('session_error', {'message': 'Session not found'})
        return

    if len(sessions[session_id]) >= 2:
        print(f"[ERROR] Session {session_id} is full")
        emit('session_error', {'message': 'Session is full'})
        return

    sessions[session_id].append(request.sid)
    join_room(session_id)
    print(f"[JOINED] {request.sid} joined session {session_id}")
    emit('session_joined', {'sessionId': session_id, 'status': 'Connected!'})
    emit('partner_connected', {'sessionId': session_id}, room=session_id)

@socketio.on('send_link_data')
def send_link_data(data):
    session_id      = data.get('sessionId')
    byte            = data.get('byte')
    current_time    = time.time()

    print(f"[SEND BYTE] {request.sid} sending byte {byte} to session {session_id}")

    if session_id not in sessions:
        print(f"[ERROR] Invalid session ID: {session_id}")
        return

    clients = sessions[session_id]
    if len(clients) != 2:
        print(f"[SKIPPED] Only one client in session {session_id}, byte not sent")
        return

    state = session_states[session_id]
    recipient = [sid for sid in clients if sid != request.sid][0]

    # - - - Check if this looks like a master request (first byte in a transfer)
    if not state.waiting_for_response:
        # - - - This is likely a master starting a transfer
        state.waiting_for_response  = True
        state.master_client         = request.sid
        state.master_byte           = byte
        state.last_transfer_time    = current_time

        print(f"[MASTER REQUEST] {request.sid} -> {recipient}, byte: {byte}")
        emit('receive_link_data', {'byte': byte}, room=recipient)

    # - - - Same client sending again - might be a race condition or retry
    elif state.master_client == request.sid:
        print(f"[MASTER RETRY] {request.sid} sent byte {byte} again, ignoring")

    # - - - This should be the slave's response
    else:
        state.waiting_for_response  = False
        state.master_client         = None

        print(f"[SLAVE RESPONSE] {request.sid} -> {recipient}, byte: {byte}")
        emit('receive_link_data', {'byte': byte}, room=recipient)

    # - - - Timeout cleanup (reset state if transfer takes too long)
    if current_time - state.last_transfer_time > 5.0:  # 5 second timeout
        print(f"[TIMEOUT] Resetting session {session_id} state")
        state.waiting_for_response  = False
        state.master_client         = None

if __name__ == '__main__':
    print("[STARTING] Server running at http://0.0.0.0:5000")
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)