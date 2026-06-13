import cv2
import numpy as np
import sys
import os

# --- OpenCV Query Tokenizer ---
VOCAB_FUR = {
    "brown": (91, 63, 51),
    "golden": (180, 130, 50),
    "dark": (30, 20, 15),
    "pink": (220, 150, 180)
}

VOCAB_EYES = {
    "green": (0, 255, 0),
    "red": (255, 0, 0),
    "blue": (0, 0, 255),
    "amber": (255, 191, 0)
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
def validate_bear_image(image_path, targets):
    print(f"[OpenCV] Loading image for visual validation: {image_path}")
    if not os.path.exists(image_path):
        print(f"[Error] Image file not found at {image_path}")
        return False
        
    img = cv2.imread(image_path)
    if img is None:
        print("[Error] Failed to parse image.")
        return False
        
    h, w, _ = img.shape
    
    # 1. Fur Color Validation (Sample central region where bear is)
    center_roi = img[int(h*0.35):int(h*0.65), int(w*0.35):int(w*0.65)]
    avg_bgr = np.mean(center_roi, axis=(0, 1))
    avg_rgb = (int(avg_bgr[2]), int(avg_bgr[1]), int(avg_bgr[0]))
    
    # Distance to target fur color
    fur_dist = np.linalg.norm(np.array(avg_rgb) - np.array(targets["fur_rgb"]))
    print(f"  -> Detected Fur RGB: {avg_rgb} (Target: {targets['fur_rgb']}, Distance: {fur_dist:.2f})")
    
    # 2. Eye Spot Validation (Threshold bright saturated regions for glowing eyes)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    
    # Mask for glowing elements (High brightness & saturation)
    mask = cv2.inRange(hsv, np.array([0, 100, 180]), np.array([180, 255, 255]))
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    detected_eyes_rgb = []
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if 5 < area < 800: # Filter typical eye sizes
            # Sample color from contour
            cnt_mask = np.zeros(mask.shape, dtype=np.uint8)
            cv2.drawContours(cnt_mask, [cnt], -1, 255, -1)
            mean_val = cv2.mean(img, mask=cnt_mask)
            eye_rgb = (int(mean_val[2]), int(mean_val[1]), int(mean_val[0]))
            detected_eyes_rgb.append(eye_rgb)
            
    print(f"  -> Detected Glowing Eye Contours: {len(detected_eyes_rgb)}")
    for idx, col in enumerate(detected_eyes_rgb):
        print(f"     Eye {idx+1} RGB: {col}")
        
    # Check if we matched color
    eye_match = False
    if detected_eyes_rgb:
        for eye_col in detected_eyes_rgb:
            dist = np.linalg.norm(np.array(eye_col) - np.array(targets["eyes_rgb"]))
            if dist < 120:
                eye_match = True
                print(f"  -> [PASS] Eye color matches target {targets['eyes_rgb']} (distance {dist:.2f})")
                break
                
    # 3. Symmetry Check
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    flipped = cv2.flip(gray, 1)
    diff = cv2.absdiff(gray, flipped)
    symmetry_score = 1.0 - (np.mean(diff) / 255.0)
    print(f"  -> Vertical Symmetry Score: {symmetry_score:.4f}")
    
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
        
    if passed:
        print("=== [SUCCESS] OpenCV Validation Passed ===")
    else:
        print("=== [FAILED] OpenCV Validation Failed ===")
        
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
