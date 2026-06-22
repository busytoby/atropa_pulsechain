import os
import sys
import time
import subprocess
import cv2
import numpy as np

def run_cmd(cmd):
    try:
        return subprocess.check_output(cmd, shell=True).decode('utf-8').strip()
    except Exception as e:
        print(f"[ERROR] Failed to run: {cmd}\n{e}")
        return ""

def main():
    print("=== Auncient Resizing Telemetry & Video Capture Test ===")
    
    # 1. Ensure temp directory for frames
    frames_dir = "/dev/shm/resize_frames"
    os.makedirs(frames_dir, exist_ok=True)
    # Clear old frames
    for f in os.listdir(frames_dir):
        if f.endswith(".jpg"):
            os.remove(os.path.join(frames_dir, f))
            
    # 2. Focus the presenter window
    print("[1/5] Focusing window class:auncient-web...")
    run_cmd("hyprctl dispatch focuswindow class:auncient-web")
    time.sleep(0.5)
    
    # Let's verify the window is focused or present
    clients = run_cmd("hyprctl clients")
    if "auncient-web" not in clients:
        print("[ERROR] auncient-web window not found in Hyprland client list.")
        sys.exit(1)
        
    print("[2/5] Starting active resize loop and capturing frames...")
    
    # We will alternate shrinking and expanding
    # resizeactive takes: delta_x delta_y
    steps = [
        (-20, -15), (-20, -15), (-20, -15), (-20, -15), (-20, -15),
        (-20, -15), (-20, -15), (-20, -15), (-20, -15), (-20, -15),
        (20, 15), (20, 15), (20, 15), (20, 15), (20, 15),
        (20, 15), (20, 15), (20, 15), (20, 15), (20, 15),
    ]
    
    frame_idx = 0
    latest_frame_path = "/dev/shm/atropa_latest_frame.jpg"
    
    for i, (dx, dy) in enumerate(steps):
        # Resize window via Hyprland
        run_cmd(f"hyprctl dispatch resizeactive {dx} {dy}")
        # Wait a tiny bit for the frame to render and get captured
        time.sleep(0.1)
        
        # Read the latest frame from memory
        if os.path.exists(latest_frame_path):
            img = cv2.imread(latest_frame_path)
            if img is not None:
                out_path = os.path.join(frames_dir, f"frame_{frame_idx:03d}.jpg")
                cv2.imwrite(out_path, img)
                frame_idx += 1
                
    print(f"Captured {frame_idx} frames during resize operation.")
    if frame_idx == 0:
        print("[ERROR] No frames captured. Is the browser controller running and writing to /dev/shm/atropa_latest_frame.jpg?")
        sys.exit(1)
        
    # 3. Perform OpenCV visual validation on one of the captured mid-resize frames
    print("[3/5] Performing Auncient tsfi_vision validation on mid-resize frame...")
    mid_frame_path = os.path.join(frames_dir, f"frame_{frame_idx // 2:03d}.jpg")
    if os.path.exists(mid_frame_path):
        img = cv2.imread(mid_frame_path)
        h, w, c = img.shape
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        mean_val = np.mean(gray)
        contrast = np.std(gray)
        sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
        
        print(f"  |- Frame dimensions: {w}x{h}")
        print(f"  |- Average pixel intensity: {mean_val:.2f}")
        print(f"  |- Standard Deviation Contrast: {contrast:.2f}")
        print(f"  |- Edge Detail Sharpness (Laplacian Var): {sharpness:.2f}")
        
        # Assertions to ensure no black screens/low contrast/blurry glitches
        if mean_val < 5.0:
            print("[FAIL] Visual validation failed: Window is pitch black (blank).")
            sys.exit(1)
        if contrast < 10.0:
            print("[FAIL] Visual validation failed: Low contrast, rendering may be broken.")
            sys.exit(1)
        if sharpness < 5.0:
            print("[FAIL] Visual validation failed: Outlines are too blurry or absent.")
            sys.exit(1)
        print("[SUCCESS] OpenCV tsfi_vision validation: PASS")
    else:
        print("[ERROR] Mid-resize frame missing.")
        sys.exit(1)
        
    # 4. Compile video using ffmpeg
    print("[4/5] Compiling video test via ffmpeg...")
    video_output = "/home/mariarahel/.gemini/antigravity-cli/brain/0c1327c6-9940-49a5-bfed-3fc06dac89c2/resize_test.mp4"
    ffmpeg_cmd = f"ffmpeg -y -framerate 10 -i {frames_dir}/frame_%03d.jpg -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -c:v libx264 -pix_fmt yuv420p {video_output}"
    run_cmd(ffmpeg_cmd)
    
    if os.path.exists(video_output) and os.path.getsize(video_output) > 0:
        print(f"[SUCCESS] Video successfully saved to: {video_output}")
    else:
        print("[ERROR] Failed to compile video file.")
        sys.exit(1)
        
    # 5. Clean up temp frames from /dev/shm
    print("[5/5] Cleaning up temporary frame sequences...")
    for f in os.listdir(frames_dir):
        if f.endswith(".jpg"):
            os.remove(os.path.join(frames_dir, f))
    os.rmdir(frames_dir)
    print("=== Resizing Telemetry Test Complete ===")

if __name__ == "__main__":
    main()
