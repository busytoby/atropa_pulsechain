import pytchat
import time
import os
import sys

# Force unbuffered output so we can see the logs instantly
sys.stdout.reconfigure(line_buffering=True)

VID_FILE = "live_video_id.txt"

def run_chat_daemon():
    print("[INFO] Chat Critic Daemon idling. Waiting for Video ID in live_video_id.txt...")
    current_chat = None
    current_vid = None
    
    while True:
        try:
            if not os.path.exists(VID_FILE):
                time.sleep(2)
                continue
                
            with open(VID_FILE, "r") as f:
                new_vid = f.read().strip()
                
            if not new_vid:
                time.sleep(2)
                continue
                
            if new_vid != current_vid or current_chat is None or not current_chat.is_alive():
                print(f"[INFO] Connecting to Chat Stream for Video ID: {new_vid}")
                current_vid = new_vid
                current_chat = pytchat.create(video_id=current_vid)
                
            if current_chat.is_alive():
                for c in current_chat.get().sync_items():
                    print(f"[CHAT] {c.author.name}: {c.message}")
                    # Write latest chat to a file for other systems to read
                    with open("latest_chat.txt", "w") as f:
                        f.write(f"{c.author.name}: {c.message}")
            else:
                print("[WARN] Chat stream disconnected or ended.")
                time.sleep(5)
                
            time.sleep(1) # Poll every second
            
        except Exception as e:
            print(f"[ERROR] Chat Daemon Exception: {e}")
            time.sleep(5)

if __name__ == "__main__":
    run_chat_daemon()
