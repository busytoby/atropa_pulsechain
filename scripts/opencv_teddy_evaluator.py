import cv2
import numpy as np
import sys
import os

# --- OpenCV Query Tokenizer ---
VOCAB_FUR = {
    "brown": (91, 63, 51),
    "golden": (180, 130, 50),
    "dark": (30, 20, 15),
    "pink": (220, 150, 180),
    "white": (240, 240, 240),
    "black": (15, 15, 15),
    "purple": (130, 50, 180),
    "orange": (230, 110, 30),
    "yellow": (220, 200, 30)
}

VOCAB_EYES = {
    "green": (0, 255, 0),
    "red": (255, 0, 0),
    "blue": (0, 0, 255),
    "amber": (255, 191, 0),
    "purple": (160, 32, 240),
    "yellow": (255, 255, 0),
    "white": (255, 255, 255),
    "orange": (255, 165, 0)
}

def tokenize_query(query_str):
    """
    Tokenizes the query string and extracts target parameters.
    """
    query_lower = query_str.lower()
    
    # Defaults
    target_fur = (91, 63, 51)
    target_eyes = (0, 255, 0)
    target_sick = 0.5
    
    for word, rgb in VOCAB_FUR.items():
        if word in query_lower:
            target_fur = rgb
            break
            
    for word, rgb in VOCAB_EYES.items():
        if word in query_lower:
            target_eyes = rgb
            break
            
    if "sick" in query_lower or "mutation" in query_lower:
        target_sick = 0.6
    elif "healthy" in query_lower or "clean" in query_lower:
        target_sick = 0.0
        
    return {
        "fur_rgb": target_fur,
        "eyes_rgb": target_eyes,
        "sickness": target_sick
    }

# --- OpenCV Visual Validator ---
def validate_bear_image(image_path, targets, print_logs=True):
    if print_logs:
        print(f"[OpenCV] Loading image for visual validation: {image_path}")
    if not os.path.exists(image_path):
        print(f"[Error] Image file not found at {image_path}")
        return False
        
    img = cv2.imread(image_path)
    if img is None:
        print("[Error] Failed to parse image.")
        return False
        
    h, w, _ = img.shape
    if w == 1280 and h == 720:
        # Crop to the teddy bear viewport (left 800x720) to exclude UI panel sidebar
        img = img[:, :800]
        h, w, _ = img.shape
    
    # 1. Fur Color Validation (Sample central region where bear is)
    center_roi = img[int(h*0.35):int(h*0.65), int(w*0.35):int(w*0.65)]
    pixels = center_roi.reshape(-1, 3)
    non_bg_pixels = pixels[np.any(pixels > 35, axis=1)]
    if len(non_bg_pixels) > 0:
        avg_bgr = np.mean(non_bg_pixels, axis=0)
    else:
        avg_bgr = np.mean(pixels, axis=0)
    avg_rgb = (int(avg_bgr[2]), int(avg_bgr[1]), int(avg_bgr[0]))
    
    # Distance to target fur color
    fur_dist = np.linalg.norm(np.array(avg_rgb) - np.array(targets["fur_rgb"]))
    print(f"  -> Detected Fur RGB: {avg_rgb} (Target: {targets['fur_rgb']}, Distance: {fur_dist:.2f})")
    
    # 2. Eye Spot Validation (Threshold bright saturated regions matching target eye color)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # Map target eye RGB to HSV ranges for precise contour isolation
    target_eyes = targets["eyes_rgb"]
    is_red = False
    if target_eyes == (0, 255, 0):    # Green
        lower_hsv = np.array([35, 50, 80])
        upper_hsv = np.array([85, 255, 255])
    elif target_eyes == (255, 0, 0):  # Red
        is_red = True
        lower_hsv1 = np.array([0, 50, 80])
        upper_hsv1 = np.array([10, 255, 255])
        lower_hsv2 = np.array([170, 50, 80])
        upper_hsv2 = np.array([180, 255, 255])
    elif target_eyes == (0, 0, 255):  # Blue
        lower_hsv = np.array([100, 50, 80])
        upper_hsv = np.array([140, 255, 255])
    elif target_eyes == (255, 191, 0): # Amber
        lower_hsv = np.array([12, 50, 80])
        upper_hsv = np.array([25, 255, 255])
    elif target_eyes == (255, 255, 255): # White
        lower_hsv = np.array([0, 0, 200])
        upper_hsv = np.array([180, 40, 255])
    elif target_eyes == (160, 32, 240):  # Purple
        lower_hsv = np.array([140, 40, 80])
        upper_hsv = np.array([165, 255, 255])
    elif target_eyes == (255, 255, 0):   # Yellow
        lower_hsv = np.array([25, 50, 80])
        upper_hsv = np.array([35, 255, 255])
    elif target_eyes == (255, 165, 0):   # Orange
        lower_hsv = np.array([5, 50, 80])
        upper_hsv = np.array([18, 255, 255])
    else:
        # Default fallback
        lower_hsv = np.array([0, 50, 80])
        upper_hsv = np.array([180, 255, 255])

    if is_red:
        mask1 = cv2.inRange(hsv, lower_hsv1, upper_hsv1)
        mask2 = cv2.inRange(hsv, lower_hsv2, upper_hsv2)
        mask = cv2.bitwise_or(mask1, mask2)
    else:
        mask = cv2.inRange(hsv, lower_hsv, upper_hsv)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    detected_eyes_rgb = []
    detected_eyes_pos = []
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if 5 < area < 800: # Filter typical eye sizes
            # Sample color from contour
            cnt_mask = np.zeros(mask.shape, dtype=np.uint8)
            cv2.drawContours(cnt_mask, [cnt], -1, 255, -1)
            mean_val = cv2.mean(img, mask=cnt_mask)
            eye_rgb = (int(mean_val[2]), int(mean_val[1]), int(mean_val[0]))
            detected_eyes_rgb.append(eye_rgb)
            # Find centroid
            M = cv2.moments(cnt)
            if M["m00"] != 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
            else:
                cx, cy = 0, 0
            detected_eyes_pos.append((cx, cy))
            
    print(f"  -> Detected Glowing Eye Contours: {len(detected_eyes_rgb)}")
    for idx, col in enumerate(detected_eyes_rgb):
        print(f"     Eye {idx+1} RGB: {col} at Pos: {detected_eyes_pos[idx]}")
        
    # Check if we matched color using HSV range
    eye_match = False
    if detected_eyes_rgb:
        for eye_col in detected_eyes_rgb:
            # Convert RGB to BGR then HSV to check hue
            eye_bgr = np.uint8([[[eye_col[2], eye_col[1], eye_col[0]]]])
            eye_hsv = cv2.cvtColor(eye_bgr, cv2.COLOR_BGR2HSV)[0][0]
            
            # Check if hue fits within the lower/upper bounds
            h_val = eye_hsv[0]
            s_val = eye_hsv[1]
            v_val = eye_hsv[2]
            if target_eyes == (0, 255, 0) and (35 <= h_val <= 85):
                eye_match = True
            elif target_eyes == (255, 0, 0) and (h_val <= 10 or h_val >= 170):
                eye_match = True
            elif target_eyes == (0, 0, 255) and (100 <= h_val <= 140):
                eye_match = True
            elif target_eyes == (255, 191, 0) and (12 <= h_val <= 25):
                eye_match = True
            elif target_eyes == (255, 255, 255) and (s_val <= 40 and v_val >= 200):
                eye_match = True
            elif target_eyes == (160, 32, 240) and (140 <= h_val <= 165):
                eye_match = True
            elif target_eyes == (255, 255, 0) and (25 <= h_val <= 35):
                eye_match = True
            elif target_eyes == (255, 165, 0) and (5 <= h_val <= 18):
                eye_match = True
            elif target_eyes == (0, 0, 0):
                eye_match = True
            else:
                # Euclidean fallback with a wider threshold (190)
                dist = np.linalg.norm(np.array(eye_col) - np.array(targets["eyes_rgb"]))
                if dist < 190:
                    eye_match = True
            
            if eye_match:
                print(f"  -> [PASS] Eye color matches target {targets['eyes_rgb']} (Detected HSV Hue: {h_val})")
                break
                
    # 3. Symmetry Check
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    flipped = cv2.flip(gray, 1)
    diff = cv2.absdiff(gray, flipped)
    symmetry_score = 1.0 - (np.mean(diff) / 255.0)
    print(f"  -> Vertical Symmetry Score: {symmetry_score:.4f}")
    
    # 4. Aspect Ratio & Coverage Checks
    _, thresholded = cv2.threshold(gray, 35, 255, cv2.THRESH_BINARY)
    bear_contours, _ = cv2.findContours(thresholded, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    bear_aspect_ratio = 1.0
    bear_coverage_ratio = 0.0
    if bear_contours:
        largest_cnt = max(bear_contours, key=cv2.contourArea)
        bx, by, bw, bh = cv2.boundingRect(largest_cnt)
        if bh > 0:
            bear_aspect_ratio = bw / bh
        bear_coverage_ratio = cv2.contourArea(largest_cnt) / (h * w)
        print(f"  -> Bear Aspect Ratio (W/H): {bear_aspect_ratio:.4f}")
        print(f"  -> Bear Area Coverage: {bear_coverage_ratio:.4f}")
    else:
        print("  -> Bear Aspect Ratio (W/H): [UNDETECTED]")
        print("  -> Bear Area Coverage: [UNDETECTED]")

    # 5. Sharpness Check (Laplacian Variance)
    sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
    print(f"  -> Surface Detail Sharpness (Laplacian Var): {sharpness:.4f}")

    # 6. Global Contrast (Standard Deviation)
    contrast_score = np.std(gray)
    print(f"  -> Global Brightness Contrast (StdDev): {contrast_score:.4f}")

    # 7. Color Saturation Channel
    saturation_score = np.mean(hsv[:, :, 1])
    print(f"  -> Global Color Saturation (Average HSV S): {saturation_score:.4f}")

    # 8. Eye Alignment Check
    eye_alignment_ok = True
    if len(detected_eyes_pos) == 2:
        eye1, eye2 = detected_eyes_pos[0], detected_eyes_pos[1]
        vert_diff = abs(eye1[1] - eye2[1]) / float(h)
        eye_midpoint_x = (eye1[0] + eye2[0]) / 2.0
        center_dev = abs(eye_midpoint_x - (w / 2.0)) / float(w)
        print(f"  -> Eye Vertical Offset: {vert_diff:.4f} | Center Deviation: {center_dev:.4f}")
        if vert_diff > 0.05:
            print("  [FAIL] Eyes are not vertically aligned.")
            eye_alignment_ok = False
        if center_dev > 0.10:
            print("  [FAIL] Eyes midpoint is significantly off-center.")
            eye_alignment_ok = False

    # Decision Matrix
    passed = True
    if fur_dist > 150:
        print("  [FAIL] Fur color deviation too high.")
        passed = False
    if targets["eyes_rgb"] != (0, 0, 0) and not eye_match:
        print("  [FAIL] Glowing eye color does not match target.")
        passed = False
    if targets["sickness"] > 0.5 and symmetry_score > 0.95:
        print("  [FAIL] Sickness mutation requested, but image is overly symmetric.")
        passed = False
    if bear_aspect_ratio < 0.5 or bear_aspect_ratio > 1.5:
        print(f"  [FAIL] Bear aspect ratio {bear_aspect_ratio:.2f} out of bound [0.5, 1.5]")
        passed = False
    if bear_coverage_ratio < 0.05 or bear_coverage_ratio > 0.85:
        print(f"  [FAIL] Bear coverage ratio {bear_coverage_ratio:.2f} out of bound [0.05, 0.85]")
        passed = False
    if sharpness < 10.0:
        print(f"  [FAIL] Bear detail sharpness {sharpness:.2f} too low (min: 10.0)")
        passed = False
    if contrast_score < 12.0:
        print(f"  [FAIL] Image contrast {contrast_score:.2f} too low (min: 12.0)")
        passed = False
    if targets["fur_rgb"] not in [(240, 240, 240), (15, 15, 15)] and saturation_score < 20.0:
        print(f"  [FAIL] Image saturation {saturation_score:.2f} too low (min: 20.0)")
        passed = False
    if not eye_alignment_ok:
        print("  [FAIL] Glowing eye alignment failed.")
        passed = False
        
    if passed:
        print("=== [SUCCESS] OpenCV Validation Passed ===")
    else:
        print("=== [FAILED] OpenCV Validation Failed ===")
        
    import json
    metrics = {
        "fur_rgb": avg_rgb,
        "fur_dist": float(fur_dist),
        "eye_count": len(detected_eyes_rgb),
        "eyes_rgb": detected_eyes_rgb[0] if detected_eyes_rgb else (0, 0, 0),
        "eye_match": bool(eye_match),
        "symmetry_score": float(symmetry_score),
        "bear_aspect_ratio": float(bear_aspect_ratio),
        "bear_coverage_ratio": float(bear_coverage_ratio),
        "sharpness": float(sharpness),
        "contrast": float(contrast_score),
        "saturation": float(saturation_score),
        "passed": bool(passed)
    }
    print(f"__METRICS_JSON__:{json.dumps(metrics)}")
        
    return passed

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 opencv_teddy_evaluator.py '<query>' <image_path>")
        sys.exit(1)
        
    query = sys.argv[1]
    img_path = sys.argv[2]
    
    targets = tokenize_query(query)
    print(f"=== OpenCV Tokenizer Targets for query '{query}': ===")
    print(f"  Fur: {targets['fur_rgb']} | Eyes: {targets['eyes_rgb']} | Sickness: {targets['sickness']}")
    
    validate_bear_image(img_path, targets)
