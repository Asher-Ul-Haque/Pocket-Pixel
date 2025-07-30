import eventlet
eventlet.monkey_patch()

from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit, join_room
import uuid

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", transports=["websocket"])
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
    for sid, clients in list(sessions.items()):
        if request.sid in clients:
            clients.remove(request.sid)
            print(f"[LEAVE] {request.sid} removed from session {sid}")
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
    session_id = data.get('sessionId')
    byte = data.get('byte')
    print(f"[SEND BYTE] {request.sid} sending byte {byte} to session {session_id}")

    if session_id not in sessions:
        print(f"[ERROR] Invalid session ID: {session_id}")
        return

    clients = sessions[session_id]
    if len(clients) == 2:
        recipient = [sid for sid in clients if sid != request.sid][0]
        emit('receive_link_data', {'byte': byte}, room=recipient)
        print(f"[FORWARDED] Byte {byte} sent from {request.sid} to {recipient}")
    else:
        print(f"[SKIPPED] Only one client in session {session_id}, byte not sent")

if __name__ == '__main__':
    print("[STARTING] Server running at http://0.0.0.0:5000")
    socketio.run(app, host='0.0.0.0', port=5000)
