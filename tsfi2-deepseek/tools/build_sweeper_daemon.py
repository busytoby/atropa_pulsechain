import os
import subprocess
import time
import socket
import json
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# This Daemon acts as the physical AST sandbox.
# It watches the 'sandbox/' directory for LLM-generated C files.
class BuildSweeper(FileSystemEventHandler):
    def on_modified(self, event):
        if not event.src_path.endswith(".c"):
            return
            
        time.sleep(0.5) # Wait for LLM file write to finalize
        filename = os.path.basename(event.src_path)
        target_src = f"src/{filename}"
        
        print(f"\\n[SWEEPER] Intercepted generated geometry: {filename}")
        
        # Ensure the original file exists in the tree before we blindly overwrite
        if not os.path.exists(target_src):
            print(f"[SWEEPER] Rejecting isolated artifact (No corresponding source): {filename}")
            return
            
        print(f"[SWEEPER] Injecting {filename} into the main TSFi architecture...")
        subprocess.run(f"cp {event.src_path} {target_src}", shell=True)
        
        # Execute the absolute GCC constraints
        print("[SWEEPER] Invoking GCC Compiler Lock...")
        result = subprocess.run("make all", shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("[SWEEPER] [PASS] Mathematical Rigidity Confirmed. Build is absolute.")
            self.broadcast_vtube(f"Build Sweeper Daemon verified the structural integrity of {filename}. Compilation successful. The architecture is locked.")
        else:
            print("[SWEEPER] [FRACTURE] Compilation Failed! Syntax Hallucination Detected.")
            
            # Extract the raw compiler error trace (first 5 lines to avoid token spam)
            error_trace = "\\n".join(result.stderr.split('\\n')[:5])
            print(error_trace)
            
            # Revert the physical architecture to its last known absolute state
            print(f"[SWEEPER] Executing git checkout to purge hallucination from {target_src}...")
            subprocess.run(f"git checkout {target_src}", shell=True)
            
            # Pipe the error back into the DeepSeek VTUBE ingestion queue via TCP 10043
            rejection_prompt = f"SYSTEM REJECTION: Your rewrite of {filename} caused a fatal GCC compiler error. Fix your syntax. Error: {error_trace}"
            self.trigger_deepseek_fix(rejection_prompt)

    def broadcast_vtube(self, text):
        with open("tools/AUDIT_FEED.md", "w") as af:
            af.write(f"=== SWEEPER DAEMON VERIFICATION ===\\n{text}\\n")

    def trigger_deepseek_fix(self, error_prompt):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect(('127.0.0.1', 10043))
            s.sendall(error_prompt.encode('utf-8'))
            s.close()
            print("[SWEEPER] Error trace blasted into DeepSeek Ingestion Node.")
        except Exception as e:
            print(f"[SWEEPER] Failed to reach DeepSeek TCP node: {e}")

def main():
    if not os.path.exists("sandbox"):
        os.makedirs("sandbox")
        
    print("=== TSFi Kr0wZ Build Sweeper Daemon Active ===")
    print("[INFO] Monitoring sandbox/ for physical LLM file generation.")
    
    event_handler = BuildSweeper()
    observer = Observer()
    observer.schedule(event_handler, path='sandbox/', recursive=False)
    observer.start()
    
    try:
        while True:
            time.sleep(86400)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

if __name__ == "__main__":
    main()
