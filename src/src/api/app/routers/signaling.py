# File: api/app/routers/signaling.py

import json
from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from typing import List

router = APIRouter()

class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)
        print("New connection established.")

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)
        print("Connection disconnected.")

    async def broadcast(self, message: str, sender: WebSocket):
        # In a more complex implementation, you might send a message only to a specific client.
        for connection in self.active_connections:
            if connection != sender:
                await connection.send_text(message)

manager = ConnectionManager()

@router.websocket("/signaling")
async def signaling_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True:
            message = await websocket.receive_text()
            print("Received message:", message)
            try:
                # Parse message if you're using JSON signaling
                data = json.loads(message)
            except json.JSONDecodeError:
                data = {"type": "unknown", "payload": message}
            # Broadcast to other connected clients (or handle logic to route messages to a specific peer)
            await manager.broadcast(message, sender=websocket)
    except WebSocketDisconnect:
        manager.disconnect(websocket)
