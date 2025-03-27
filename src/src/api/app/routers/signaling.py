# File: api/app/routers/signaling.py

import json
from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from typing import Dict, List

router = APIRouter()


class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []
        self.connections: Dict[str, WebSocket] = {}  # Maps client_id to WebSocket

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)
        print("New connection established.")

    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)
        # Remove the client from the mapping if present.
        removed_ids = [client_id for client_id, ws in self.connections.items() if ws == websocket]
        for client_id in removed_ids:
            del self.connections[client_id]
            print(f"Connection for client '{client_id}' removed.")
        print("Connection disconnected.")

    async def register_client(self, client_id: str, websocket: WebSocket):
        self.connections[client_id] = websocket
        print(f"Registered client: {client_id}")

    async def send_to_client(self, client_id: str, message: str):
        if client_id in self.connections:
            ws = self.connections[client_id]
            await ws.send_text(message)
            print(f"Sent message to {client_id}: {message}")
        else:
            print(f"Client {client_id} not found.")

    async def broadcast(self, message: str, sender: WebSocket):
        to_remove = []
        for connection in self.active_connections:
            if connection != sender:
                try:
                    await connection.send_text(message)
                except RuntimeError as e:
                    # Log the error and mark the connection for removal.
                    print(
                        f"Error when sending broadcast message: {e}. Removing connection."
                    )
                    to_remove.append(connection)
        for connection in to_remove:
            if connection in self.active_connections:
                self.active_connections.remove(connection)
        print("Broadcasted message:", message)


manager = ConnectionManager()


@router.websocket("/signaling")
async def signaling_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        # Expect a registration message as the first message:
        # e.g. {"type": "register", "id": "client1"}
        initial_data = await websocket.receive_text()
        try:
            data = json.loads(initial_data)
            if data.get("type") == "register" and data.get("id"):
                await manager.register_client(data["id"], websocket)
            else:
                await websocket.send_text(
                    json.dumps({"error": "First message must be registration with a client id."})
                )
                await websocket.close()
                return
        except json.JSONDecodeError:
            await websocket.send_text(json.dumps({"error": "Invalid JSON in registration."}))
            await websocket.close()
            return

        # Process subsequent messages.
        while True:
            message = await websocket.receive_text()
            print("Received message:", message)
            try:
                data = json.loads(message)
            except json.JSONDecodeError:
                data = {"type": "unknown", "payload": message}

            # Extract message type and optional target client id.
            msg_type = data.get("type")
            target_id = data.get("target")

            # For signaling types, route appropriately.
            if msg_type in ("offer", "answer", "ice-candidate", "test"):
                if target_id:
                    # Send to the specific target client.
                    await manager.send_to_client(target_id, message)
                else:
                    # If no target is specified, broadcast to all other clients.
                    await manager.broadcast(message, sender=websocket)
            else:
                # For any unknown message types, you can either echo
                # or ignore. Here we choose to echo.
                await websocket.send_text(message)
    except WebSocketDisconnect:
        manager.disconnect(websocket)
