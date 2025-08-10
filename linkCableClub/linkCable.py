import eventlet
eventlet.monkey_patch()
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit, join_room, close_room
import uuid
import time

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", transports=["websocket"])

# Dictionary to store active sessions.
sessions                    = {}
sessions_with_pending_bytes = {}

@app.route('/')
def index():
  return render_template('index.html')


@socketio.on('connect')
def on_connect():
  print(f"[CONNECT] Client connected: {request.sid}")


@socketio.on('exit')
def on_disconnect():
  print(f"[DISCONNECT] Client disconnected: {request.sid}")

  # Find which session this client was part of
  for session_id, session_data in list(sessions.items()):
    clients_in_session = session_data['clients']
    if request.sid in clients_in_session:
      clients_in_session.remove(request.sid)
      print(f"[LEAVE] {request.sid} removed from session {session_id}")

      # Notify remaining client if there's only one left
      if len(clients_in_session) == 1:
        remaining_client_sid = list(clients_in_session)[0]
        emit("partner_disconnected", room=remaining_client_sid)
        print(f"[NOTIFY] Partner disconnected in session {session_id}, notifying {remaining_client_sid}")

        # Clear master/slave roles since the session is now invalid
        session_data['master']  = None
        session_data['slave']   = None

      # If no clients left, remove the session
      elif not clients_in_session:
        del sessions[session_id]
        close_room(session_id)
        print(f"[SESSION REMOVED] {session_id}")

      # Also clear any pending transfers for this session
      if session_id in sessions_with_pending_bytes:
        del sessions_with_pending_bytes[session_id]
        print(f"[CLEANUP] Cleared pending transfer for session {session_id}")
      break


@socketio.on('create_session')
def on_create_session():
  # Generate a new unique session ID
  session_id                        = str(uuid.uuid4())[:8]
  sessions[session_id]              = {'clients': set(), 'master': None, 'slave': None}
  sessions[session_id]['clients'].add(request.sid)
  sessions[session_id]['master']    = request.sid

  join_room(session_id)
  print(f"[CREATE] New session created: {session_id} by {request.sid}. Assigned as Master.")
  emit('session_created', {'session_id': session_id}, room=request.sid)
  emit('waiting_for_partner', {'session_id': session_id}, room=request.sid)


@socketio.on('join_session')
def on_join_session(data):
  session_id = data.get('session_id')

  # Generate a new session ID if none provided
  if not session_id or session_id not in sessions:
    print(f"[REJECT] Session {session_id} not found. {request.sid} rejected.")
    emit('session_not_found', room=request.sid)
    return

  clients_in_session = sessions[session_id]['clients']
  if len(clients_in_session) < 2:
    join_room(session_id)
    clients_in_session.add(request.sid)
    sessions[session_id]['slave'] = request.sid
    print(f"[JOIN] {request.sid} joined session {session_id}. Assigned as Slave. Current clients: {len(clients_in_session)}")

    # Notify both clients that the session is ready
    if len(clients_in_session) == 2:
      print(f"[READY] Session {session_id} is full. Both clients connected.")
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

  # Find the session and role for this client
  session_id    = None
  client_type   = None
  for sid, session_data in sessions.items():
    if request.sid == session_data.get('master'):
      session_id    = sid
      client_type   = "master"
      break
    elif request.sid == session_data.get('slave'):
      session_id    = sid
      client_type   = "slave"
      break

  if not session_id:
    print(f"[ERROR] {request.sid} not in any active session, ignoring data.")
    return

  # Initialize a pending transfer object if one doesn't exist for the session
  print(f"[TRANSFER] Received byte from {client_type} ({request.sid}), session {session_id}. Byte: 0x{byte:02X}")
  if session_id not in sessions_with_pending_bytes:
    sessions_with_pending_bytes[session_id] = {}

  # Store the incoming byte based on its client type (master or slave)
  sessions_with_pending_bytes[session_id][client_type] = {'sid': request.sid, 'byte': byte}

  # Check if both a master and a slave byte are now available for this session
  if 'master' in sessions_with_pending_bytes[session_id] and 'slave' in sessions_with_pending_bytes[session_id]:
    master_data = sessions_with_pending_bytes[session_id]['master']
    slave_data  = sessions_with_pending_bytes[session_id]['slave']
    master_sid  = master_data['sid']
    master_byte = master_data['byte']
    slave_sid   = slave_data['sid']
    slave_byte  = slave_data['byte']

    # Send the master's byte to the slave
    emit('receive_link_data', {'byte': master_byte}, room=slave_sid)
    print(f"[RELAY] Master ({master_sid}) -> Slave ({slave_sid}), byte: 0x{master_byte:02X}")

    # Send the slave's byte to the master
    emit('receive_link_data', {'byte': slave_byte}, room=master_sid)
    print(f"[RELAY] Slave ({slave_sid}) -> Master ({master_sid}), byte: 0x{slave_byte:02X}")

    # Clear the pending transfer for this session
    del sessions_with_pending_bytes[session_id]
    print(f"[SYNC] Transfer complete for session {session_id}. Cleared pending transfer.")

  else:
    print(f"[PENDING] Stored byte from {client_type} ({request.sid}), waiting for partner.")
    emit('waiting_for_transfer_partner', room=request.sid)


if __name__ == '__main__':
  print("Starting Flask-SocketIO server for Game Boy Link Cable emulation...")
  socketio.run(app, host='0.0.0.0', port=5000, debug=True)
