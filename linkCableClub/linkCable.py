# app.py
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit, join_room, leave_room
import uuid
import os

app = Flask(__name__)
# Configure SocketIO for WebSocket communication
# For production, you'd use a real message queue like Redis:
# app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'your_secret_key_here')
# app.config['DEBUG'] = True # Set to False in production
# socketio = SocketIO(app, message_queue='redis://localhost:6379/0')
# For local testing, no message queue is needed:
socketio = SocketIO(app, cors_allowed_origins="*") # Allow all origins for development

# Dictionary to store active rooms (sessions)
# Key: session_id (string), Value: list of connected SIDs (session IDs for SocketIO clients)
active_sessions = {}

@app.route('/')
def index():
    """Serve the main HTML client page."""
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    """Handle new client connections."""
    print(f"Client connected: {request.sid}")

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnections."""
    print(f"Client disconnected: {request.sid}")
    # Remove client from any sessions they were in
    for session_id, sids in list(active_sessions.items()):
        if request.sid in sids:
            sids.remove(request.sid)
            print(f"Client {request.sid} left session {session_id}")
            # If a session becomes empty, remove it
            if not sids:
                del active_sessions[session_id]
                print(f"Session {session_id} is now empty and removed.")
            # Notify the other client if there was one
            elif len(sids) == 1:
                emit('partner_disconnected', room=sids[0])

@socketio.on('create_session')
def create_session():
    """Create a new link session and join it."""
    session_id = str(uuid.uuid4())[:8] # Generate a short, unique ID
    active_sessions[session_id] = [request.sid]
    join_room(session_id)
    print(f"Client {request.sid} created and joined session: {session_id}")
    emit('session_created', {'sessionId': session_id, 'status': 'Waiting for partner...'})

@socketio.on('join_session')
def join_session(data):
    """Join an existing link session."""
    session_id = data.get('sessionId')
    if not session_id or session_id not in active_sessions:
        emit('session_error', {'message': 'Session not found or invalid ID.'})
        print(f"Client {request.sid} failed to join session {session_id}: Not found.")
        return

    sids_in_room = active_sessions[session_id]
    if len(sids_in_room) >= 2:
        emit('session_error', {'message': 'Session is full.'})
        print(f"Client {request.sid} failed to join session {session_id}: Full.")
        return

    sids_in_room.append(request.sid)
    join_room(session_id)
    print(f"Client {request.sid} joined session: {session_id}")
    emit('session_joined', {'sessionId': session_id, 'status': 'Connected!'})

    # Notify both clients that they are connected
    emit('partner_connected', {'sessionId': session_id}, room=session_id)

@socketio.on('send_link_data')
def send_link_data(data):
    """Relay Game Boy link cable data between clients in a session."""
    session_id = data.get('sessionId')
    byte_data = data.get('byte') # Assuming byte is sent as an integer (0-255)

    if not session_id or session_id not in active_sessions:
        # This shouldn't happen if clients are properly managed, but good for robustness
        print(f"Error: Link data for unknown session {session_id} from {request.sid}")
        return

    sids_in_room = active_sessions[session_id]
    if len(sids_in_room) == 2:
        # Find the other client in the room and send them the data
        other_sid = [sid for sid in sids_in_room if sid != request.sid]
        if other_sid:
            emit('receive_link_data', {'byte': byte_data}, room=other_sid[0])
            # print(f"Relayed byte {byte_data} from {request.sid} to {other_sid[0]} in {session_id}")
    # else:
        # print(f"Warning: Link data sent to session {session_id} with < 2 clients. Data: {byte_data}")

if __name__ == '__main__':
    # For local development:
    # To run: python app.py
    # Then open http://127.0.0.1:5000 in your browser (two tabs for testing)
    socketio.run(app, debug=True, host='0.0.0.0', port=os.environ.get('PORT', 5000))

    # For deployment (e.g., on Render.com, Heroku), they will use the Procfile
