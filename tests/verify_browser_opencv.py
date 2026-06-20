import cv2
import numpy as np
import sys
import os

def main():
    print("=== Auncient OpenCV tsfi_vision Validator ===")
    img_path = "frontend/latest_frame.jpg"
    if not os.path.exists(img_path):
        print(f"[FAIL] Screenshot {img_path} not found.")
        sys.exit(1)
        
    img = cv2.imread(img_path)
    if img is None:
        print("[FAIL] Failed to load screenshot via OpenCV.")
        sys.exit(1)
        
    h, w, c = img.shape
    print(f"[INFO] Loaded screenshot dimensions: {w}x{h} with {c} channels.")
    
    # 1. Non-black check (Ensure screen is not blank)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    mean_val = np.mean(gray)
    print(f"  |- Average pixel intensity: {mean_val:.2f}")
    if mean_val < 5.0:
        print("[FAIL] Visual validation failed: Window is pitch black (blank).")
        sys.exit(1)
        
    # 2. Contrast check (Ensure text/shapes are rendered)
    contrast = np.std(gray)
    print(f"  |- Standard Deviation Contrast: {contrast:.2f}")
    if contrast < 10.0:
        print("[FAIL] Visual validation failed: Low contrast, rendering may be broken.")
        sys.exit(1)
        
    # 3. Component Verification (Identify shapes/colors)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # Check for red channel to detect YouTube UI elements
    lower_red1 = np.array([0, 70, 50])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([170, 70, 50])
    upper_red2 = np.array([180, 255, 255])
    
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    red_mask = cv2.bitwise_or(mask1, mask2)
    
    red_pixels = np.sum(red_mask > 0)
    print(f"  |- Detected red content pixels: {red_pixels}")
    
    # Check for magenta/purple DNA elements
    lower_magenta = np.array([130, 40, 50])
    upper_magenta = np.array([165, 255, 255])
    magenta_mask = cv2.inRange(hsv, lower_magenta, upper_magenta)
    magenta_pixels = np.sum(magenta_mask > 0)
    print(f"  |- Detected magenta/purple DNA pixels: {magenta_pixels}")

    # Check for edge detail (Laplacian)
    sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
    print(f"  |- Edge Detail Sharpness (Laplacian Var): {sharpness:.2f}")
    
    if sharpness < 5.0:
        print("[FAIL] Visual verification failed: Outlines are too blurry or absent.")
        sys.exit(1)
        
    print("[SUCCESS] Auncient OpenCV tsfi_vision Verification: PASS")

if __name__ == "__main__":
    main()
