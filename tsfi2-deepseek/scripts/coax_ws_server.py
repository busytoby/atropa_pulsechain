# Auncient Coaxial SCSI WebSockets Telemetry Server (coax_ws_server.py)
import asyncio
import json
import websockets
import math
import random

async def telemetry_loop(websocket):
    print("[WS] Client connected to coaxial telemetry stream.")
    t = 0.0
    try:
        while True:
            # 1. Modulate Lissajous frequencies based on system state
            fx = 3.0 + math.sin(t * 0.1) * 0.5
            fy = 4.0 + math.cos(t * 0.15) * 0.5
            fz = 2.0 + math.sin(t * 0.05) * 0.2
            
            # 2. Mock ZMM VM memory registers
            zmm_mem = {
                "0xF101": int(8 + math.sin(t * 2.0) * 7), # Sound volume
                "0xF300": 0xFF00FF if (int(t) % 2 == 0) else 0x00FFFF, # Palette Color
                "0xF600": random.randint(0, 255), # SCSI WinchesterMQ status
            }
            
            payload = {
                "fx": fx,
                "fy": fy,
                "fz": fz,
                "zmm": zmm_mem,
                "timestamp": t
            }
            
            await websocket.send(json.dumps(payload))
            await asyncio.sleep(0.05) # 20 FPS stream
            t += 0.05
    except websockets.exceptions.ConnectionClosed:
        print("[WS] Client disconnected.")

async def main():
    print("[WS] Starting Auncient Telemetry WebSockets Server on port 8765...")
    async with websockets.serve(telemetry_loop, "0.0.0.0", 8765):
        await asyncio.Future() # run forever

if __name__ == "__main__":
    asyncio.run(main())
