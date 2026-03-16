import cv2
import numpy as np

def analyze_plush_quality(rendered_img_path, original_prompt, decay_ratio):
    """
    The Spider's Content Evaluator: Looks *inside* the approved Alligator boundaries 
    to classify specific qualitative features of the stuffed animals.
    """
    img = cv2.imread(rendered_img_path)
    if img is None:
        return {"error": "image_not_found"}

    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    # 1. Texture/Fur Analysis (Variance of Laplacian)
    # Measures how "fuzzy" or "sharp" the plush material is.
    sharpness = cv2.Laplacian(gray, cv2.CV_64F).var()
    
    # 2. Lighting/Contrast Analysis
    # Checks if the "softbox studio lighting" is actually working.
    # If the image is too dark, it failed the lighting prompt.
    brightness = np.mean(gray)
    contrast = gray.std()
    
    # 3. Species / Material Detection (Color Masking)
    # Alligator -> Hunter Green (HSV ~40-70, lower brightness)
    lower_green = np.array([35, 40, 40])
    upper_green = np.array([85, 255, 200])
    mask_gator = cv2.inRange(hsv, lower_green, upper_green)
    gator_pixels = cv2.countNonZero(mask_gator)
    
    # Teddy Bear -> Brown/Tan (HSV ~10-25)
    lower_brown = np.array([10, 50, 20])
    upper_brown = np.array([25, 255, 200])
    mask_bear = cv2.inRange(hsv, lower_brown, upper_brown)
    bear_pixels = cv2.countNonZero(mask_bear)
    
    total_pixels = img.shape[0] * img.shape[1]
    
    # 4. Synthesize the Spider's Report
    report = {
        "sharpness": float(sharpness),
        "brightness": float(brightness),
        "contrast": float(contrast),
        "gator_presence": (gator_pixels / total_pixels) * 100.0,
        "bear_presence": (bear_pixels / total_pixels) * 100.0,
        "action_required": "NONE"
    }
    
    # Evaluate Rules
    if report["brightness"] < 80.0:
        report["action_required"] = "INCREASE_LIGHTING_PROMPT"
        report["advice"] = "The scene is too dark. The softbox lighting is failing."
        
    elif report["sharpness"] < 100.0:
        report["action_required"] = "INCREASE_STEPS"
        report["advice"] = "The plush fur is blurry or melting. Increase SD steps."
        
    elif report["gator_presence"] < 2.0 and "alligator" in original_prompt.lower():
        report["action_required"] = "EMPHASIZE_GATOR"
        report["advice"] = "The green alligator plush is missing or too small. Emphasize it in the prompt."

    elif report["bear_presence"] < 5.0 and "bear" in original_prompt.lower():
        report["action_required"] = "EMPHASIZE_BEAR"
        report["advice"] = "The brown teddy bear is missing. Emphasize it in the prompt."

    return report

if __name__ == "__main__":
    report = analyze_plush_quality("assets/mpx_out.png", "a green alligator and brown teddy bear", 0.0)
    for k, v in report.items():
        print(f"{k}: {v}")
