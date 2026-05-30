import asyncio
import json
import random
import string
import time
import os
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse

app = FastAPI()

rooms = {}

def generate_room_code(length=5):
    characters = string.ascii_uppercase + string.digits
    while True:
        code = ''.join(random.choices(characters, k=length))
        if code not in rooms:
            return code

async def cleanup_stale_rooms():
    while True:
        await asyncio.sleep(60)
        current_time = time.time()
        stale_codes = []
        
        for code, room in rooms.items():
            if room.get("p2") is None and (current_time - room["created_at"]) > 300:
                stale_codes.append(code)
                
        for code in stale_codes:
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
    asyncio.create_task(cleanup_stale_rooms())

@app.websocket("/ws")
async def link_cable_endpoint(websocket: WebSocket):
    await websocket.accept()
    current_room_code = None
    player_role = None

    try:
        while True:
            message = await websocket.receive()

            if "text" in message:
                data = json.loads(message["text"])
                action = data.get("action")

                if action == "create":
                    current_room_code = generate_room_code()
                    player_role = "p1"
                    rooms[current_room_code] = {
                        "p1": websocket,
                        "p2": None,
                        "created_at": time.time(),
                        "p1_byte": None, # <--- NEW: Buffer for Player 1's byte
                        "p2_byte": None  # <--- NEW: Buffer for Player 2's byte
                    }
                    await websocket.send_json({"type": "created", "code": current_room_code})

                elif action == "join":
                    attempted_code = data.get("code", "").upper().strip()
                    if attempted_code not in rooms:
                        await websocket.send_json({"type": "error", "message": "Room not found."})
                        continue
                    
                    room = rooms[attempted_code]
                    if room["p2"] is not None:
                        await websocket.send_json({"type": "error", "message": "Room is full."})
                        continue

                    current_room_code = attempted_code
                    player_role = "p2"
                    room["p2"] = websocket
                    
                    await room["p1"].send_json({"type": "connected"})
                    await room["p2"].send_json({"type": "connected"})

                elif action == "disconnect":
                    break 

            elif "bytes" in message:
                if not current_room_code or rooms[current_room_code].get("p2") is None:
                    continue 

                room = rooms[current_room_code]
                payload = message["bytes"]

                # --- THE LOCK-STEP FIX ---
                # 1. Store the incoming byte in the correct player's buffer
                if player_role == "p1":
                    room["p1_byte"] = payload
                elif player_role == "p2":
                    room["p2_byte"] = payload

                # 2. Check if we have received a byte from BOTH players
                if room["p1_byte"] is not None and room["p2_byte"] is not None:
                    # Both games are officially waiting in their while-loops. 
                    # Swap the bytes and fire them back simultaneously!
                    try:
                        await room["p1"].send_bytes(room["p2_byte"])
                        await room["p2"].send_bytes(room["p1_byte"])
                    except:
                        pass # Handled gracefully by the disconnect block below
                    finally:
                        # Clear the buffers for the next 8-bit cycle
                        room["p1_byte"] = None
                        room["p2_byte"] = None

    except WebSocketDisconnect:
        pass
        
    finally:
        if current_room_code and current_room_code in rooms:
            room = rooms[current_room_code]
            other_role = "p2" if player_role == "p1" else "p1"
            other_ws = room.get(other_role)
            
            if other_ws:
                try:
                    await other_ws.send_json({"type": "peer_disconnected", "message": "The other player disconnected."})
                    await other_ws.close()
                except:
                    pass
            del rooms[current_room_code]

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    uvicorn.run("linkCable:app", host="0.0.0.0", port=port)
