# Example script to demonstrate how the Gemini MCP bridge reads the stream
import socket
import json
import time

def poll_mcp():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect(('127.0.0.1', 10042))
        s.sendall(b'{"method": "manifold.get_lore", "id": 999}\n')
        data = s.recv(8192)
        print(f"[Gemini CLI MCP Poller] {data.decode('utf-8').strip()}")
        s.close()
    except Exception as e:
        print(f"MCP Connection failed: {e}")

poll_mcp()
