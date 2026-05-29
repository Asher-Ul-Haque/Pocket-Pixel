import asyncio
import json
import random
import string
import time
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
import os
import uvicorn

app = FastAPI()

# The master dictionary holding our Link Cable Clubs
# Format: { "CODE": { "p1": WebSocket, "p2": WebSocket, "created_at": timestamp } }
rooms = {}

def generate_room_code(length=5):
    """Generates a random, easy-to-type alphanumeric shortcode."""
    characters = string.ascii_uppercase + string.digits
    while True:
        code = ''.join(random.choices(characters, k=length))
        if code not in rooms:
            return code

async def cleanup_stale_rooms():
    """Background daemon: Deletes rooms that haven't paired up within 5 minutes."""
    while True:
        await asyncio.sleep(60) # Check every minute
        current_time = time.time()
        stale_codes = []
        
        for code, room in rooms.items():
            # If the room is older than 5 minutes AND nobody joined as player 2
            if room.get("p2") is None and (current_time - room["created_at"]) > 300:
                stale_codes.append(code)
                
        for code in stale_codes:
            print(f"[CLEANUP] Deleting stale room: {code}")
            # Notify player 1 that the room expired
            p1_ws = rooms[code].get("p1")
            if p1_ws:
                try:
                    await p1_ws.send_json({"type": "error", "message": "Room expired after 5 minutes of inactivity."})
                    await p1_ws.close()
                except:
                    pass
            del rooms[code]

@app.on_event("startup")
async def startup_event():
    # Boot up the background cleanup daemon when the server starts
    asyncio.create_task(cleanup_stale_rooms())

@app.get("/")
async def get_frontend():
    # Serve the frontend HTML file
    with open("templates/index.html", "r") as f:
        return HTMLResponse(f.read())

@app.websocket("/ws")
async def link_cable_endpoint(websocket: WebSocket):
    await websocket.accept()
    current_room_code = None
    player_role = None # "p1" or "p2"

    try:
        while True:
            # Receive any incoming message (can be text/JSON for signaling, or raw bytes for emulation)
            message = await websocket.receive()

            # --- SIGNALING LAYER (JSON) ---
            if "text" in message:
                data = json.loads(message["text"])
                action = data.get("action")

                if action == "create":
                    current_room_code = generate_room_code()
                    player_role = "p1"
                    rooms[current_room_code] = {
                        "p1": websocket,
                        "p2": None,
                        "created_at": time.time()
                    }
                    print(f"[CLUB] Room created: {current_room_code}")
                    await websocket.send_json({"type": "created", "code": current_room_code})

                elif action == "join":
                    attempted_code = data.get("code", "").upper().strip()
                    if attempted_code not in rooms:
                        await websocket.send_json({"type": "error", "message": "Room not found."})
                        continue
                    
                    room = rooms[attempted_code]
                    if room["p2"] is not None:
                        await websocket.send_json({"type": "error", "message": "Sorry, your friend has more friends! Room is full."})
                        continue

                    # Success: Pair them up!
                    current_room_code = attempted_code
                    player_role = "p2"
                    room["p2"] = websocket
                    
                    print(f"[CLUB] Room paired: {current_room_code}")
                    # Notify both players the cable is physically "plugged in"
                    await room["p1"].send_json({"type": "connected"})
                    await room["p2"].send_json({"type": "connected"})

                elif action == "disconnect":
                    # Client requested a polite disconnect
                    break 

            # --- EMULATION LAYER (RAW BYTES) ---
            elif "bytes" in message:
                if not current_room_code or rooms[current_room_code].get("p2") is None:
                    continue # Ignore bytes if not fully paired

                room = rooms[current_room_code]
                payload = message["bytes"]

                # Route the byte to the OTHER player
                if player_role == "p1":
                    await room["p2"].send_bytes(payload)
                elif player_role == "p2":
                    await room["p1"].send_bytes(payload)

    except WebSocketDisconnect:
        print(f"[CLUB] User disconnected from room: {current_room_code}")
        
    finally:
        # --- TEARDOWN SEQUENCE ---
        if current_room_code and current_room_code in rooms:
            room = rooms[current_room_code]
            
            # Notify the OTHER player that the cable was yanked out
            other_role = "p2" if player_role == "p1" else "p1"
            other_ws = room.get(other_role)
            
            if other_ws:
                try:
                    await other_ws.send_json({"type": "peer_disconnected", "message": "The other player disconnected."})
                    await other_ws.close()
                except:
                    pass
            
            # Nuke the room dictionary
            del rooms[current_room_code]
            print(f"[CLUB] Room destroyed: {current_room_code}")
            
if __name__ == "__main__":
    # Render dynamically assigns a port via the PORT environment variable.
    # We default to 8000 so you can still test it locally.
    port = int(os.environ.get("PORT", 8000))
    
    # 0.0.0.0 exposes the server to the public internet
    uvicorn.run("linkCable:app", host="0.0.0.0", port=port)
