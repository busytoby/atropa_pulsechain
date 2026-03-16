import socket
import json
import time
import sys
import os
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class ManifoldInterruptHandler(FileSystemEventHandler):
    def __init__(self):
        self.last_lore = ""

    def trigger_mcp_pull(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect(('127.0.0.1', 10042))
            
            req = {"method": "manifold.get_lore", "id": 1}
            s.sendall((json.dumps(req) + "\n").encode('utf-8'))
            
            data = s.recv(65536)
            s.close()
            
            if data:
                resp = json.loads(data.decode('utf-8'))
                if "result" in resp and "lore" in resp["result"]:
                    current_lore = resp["result"]["lore"]
                    if current_lore != self.last_lore and current_lore != "":
                        print("\n[Kr0wZ REAL-TIME] Physical Interrupt Triggered via Zero-Copy FS Event:")
                        print("==================================================")
                        print(current_lore)
                        print("==================================================\n")
                        self.last_lore = current_lore
        except Exception as e:
            pass # Ignore connection drops

    def on_modified(self, event):
        if "audit_lore.pos" in event.src_path or "AUDIT_FEED.md" in event.src_path:
            self.trigger_mcp_pull()

def run_bridge():
    print("=== Kr0wZ Hardware-Interrupt Bridge Active (Zero Polling) ===")
    
    event_handler = ManifoldInterruptHandler()
    
    # Do an initial pull to establish baseline
    event_handler.trigger_mcp_pull()
    
    observer = Observer()
    # Watch the tools directory for physical modifications to the manifold/feed
    observer.schedule(event_handler, path='tools/', recursive=False)
    # Also watch the root directory for audit_lore.pos
    observer.schedule(event_handler, path='.', recursive=False)
    observer.start()
    
    try:
        while True:
            time.sleep(86400) # The main thread sleeps permanently. The observer thread handles interrupts.
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

if __name__ == "__main__":
    run_bridge()
