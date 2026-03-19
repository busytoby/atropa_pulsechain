import os
import time
import subprocess
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn

# --- Configuration ---
STREAM_KEY_FILE = os.path.expanduser("~/.config/gemini/stream_key.txt")

def get_stream_key():
    try:
        with open(STREAM_KEY_FILE, "r") as f:
            return f.read().strip()
    except:
        return None

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer): pass
class WorldSerpentHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200); self.send_header('Content-type', 'text/html'); self.end_headers()
            self.wfile.write(b"<html><body style='background:#000;color:#0ff;'><h1>TSFi QoS Hardened Hardware Broadcast Active (AB4H)</h1></body></html>")

def launch_hardware_broadcaster():
    key = get_stream_key()
    if not key:
        print("[FRACTURE] Missing stream key for Hardware Broadcaster.")
        return None
    
    print("[SYSTEM] Engaging Vulkan Hardware Encoder (4:4:4 AB4H Profile)...")
    env = os.environ.copy()
    env["TSFI_VALIDATION"] = "0"
    return subprocess.Popen(["bin/tsfi_native_broadcast", key], env=env)

if __name__ == '__main__':
    hw_proc = launch_hardware_broadcaster()
    if not hw_proc:
        import sys
        sys.exit(1)
        
    subprocess.run(["fuser", "-k", "9093/tcp"])
    server = ThreadedHTTPServer(('0.0.0.0', 9093), WorldSerpentHandler)
    print("=== World Serpent Distribution (Port 9093) ===")
    print("Hardware Encoder Online. Awaiting neural visual injection.")
    
    import threading
    threading.Thread(target=server.serve_forever, daemon=True).start()
    
    while True:
        if hw_proc.poll() is not None:
            print(f"[FATAL] Hardware Encoder process died with exit code {hw_proc.returncode}. Terminating World Serpent.")
            os._exit(1)
        time.sleep(1)
