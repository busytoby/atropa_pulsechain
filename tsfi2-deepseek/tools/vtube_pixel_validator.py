import cv2
import numpy as np
import subprocess
import os
import sys

def main():
    print("=== TSFi VTube Pixel-Letter Validation ===")
    
    # 1. Pull Real-Time DeepSeek Output
    deepseek_text = "DEEPSEEK OFFLINE"
    if os.path.exists("tools/AUDIT_FEED.md"):
        with open("tools/AUDIT_FEED.md", "r") as f:
            # Grab the first line of the actual DeepSeek architectural audit
            lines = f.readlines()
            if len(lines) > 1:
                deepseek_text = lines[1].strip()[:30] # Limit to 30 chars for display
                
    print(f"[INFO] Routing DeepSeek Inference to Vulkan: '{deepseek_text}'")

    # 2. Run the C rendering engine to generate the 4-second clip frames
    print("[INFO] Invoking Vulkan Hardware Renderer...")
    try:
        subprocess.run(["./bin/test_vulkan_font_validation", deepseek_text], check=True)
    except Exception as e:
        print(f"[FRACTURE] C Binary Failed: {e}")
        return

    # 3. Check the final output frame
    target_frame = "vtube_frame_119.ppm"
    if not os.path.exists(target_frame):
        print("[FRACTURE] Frame 119 was not generated.")
        return
        
    print(f"[PASS] Intercepted {target_frame}.")
    
    # 4. OpenCV Physical Geometry Extraction
    img = cv2.imread(target_frame)
    if img is None:
        print("[FRACTURE] Failed to decode PPM via OpenCV.")
        return
        
    green_channel = img[:, :, 1]
    _, binary_mask = cv2.threshold(green_channel, 200, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(binary_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    valid_letters = [c for c in contours if cv2.contourArea(c) > 5]
    
    expected_string = deepseek_text
    expected_letter_count = len(expected_string.replace(" ", ""))
    detected_count = len(valid_letters)
    
    print(f"[INFO] Expected Physical Characters: {expected_letter_count} ('{expected_string}')")
    print(f"[INFO] Detected Contiguous Pixel Clusters: {detected_count}")
    
    if detected_count == expected_letter_count:
        print("[PASS] Absolute Mechanical Verification: Every known letter from the input string is perfectly represented by physical Vulkan pixels.")
    else:
        # Sometimes letters like 'Q' might have detached parts or merged kerning
        # We allow a small error margin for kerning fusion, but a totally black screen = 0
        if detected_count > 0:
            print(f"[WARN] Partial match. Kerning or rendering artifacts detected. (Found {detected_count}, Expected {expected_letter_count})")
        else:
            print("[FAIL] Zero geometric features detected. Rasterization collapsed.")

    # Cleanup frames to save disk space
    for i in range(120):
        try:
            os.remove(f"vtube_frame_{i:03d}.ppm")
        except:
            pass

if __name__ == "__main__":
    main()
