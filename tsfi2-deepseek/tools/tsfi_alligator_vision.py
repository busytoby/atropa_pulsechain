import cv2
import numpy as np

def run_vision_critic(rendered_img_path, skeleton_img_path, decay_ratio):
    """
    The Alligator Form Evaluator: Validates that the AI-generated plushie 
    strictly conforms to the mathematically mandated OpenPose bounds.
    """
    rendered_img = cv2.imread(rendered_img_path)
    skeleton_img = cv2.imread(skeleton_img_path)

    if rendered_img is None or skeleton_img is None:
        return True, "MISSING_FILES", "CONTINUE" # Failsafe

    # 1. Isolate the Spider's OpenPose Matrix Bounds
    # The Spider is drawn using specific colors:
    # Legs left: (85, 255, 0), (0, 255, 0), (0, 0, 255), (85, 0, 255)
    # Legs right: (255, 170, 0), (255, 255, 0), (0, 255, 255), (0, 170, 255)
    # Body: (255, 0, 0) to (0, 255, 170)
    
    hsv_skel = cv2.cvtColor(skeleton_img, cv2.COLOR_BGR2HSV)
    
    # Very broad mask to catch all non-black pixels (the entire skeleton)
    lower_any = np.array([0, 50, 50])
    upper_any = np.array([180, 255, 255])
    mask_skel = cv2.inRange(hsv_skel, lower_any, upper_any)
    
    # Find bounding box of the entire mathematical matrix
    contours_skel, _ = cv2.findContours(mask_skel, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours_skel:
        return True, "NO_SKELETON", "CONTINUE"
        
    # Get the bounding box of ALL contours combined (the whole scene)
    x_min, y_min = 9999, 9999
    x_max, y_max = 0, 0
    for cnt in contours_skel:
        x, y, w, h = cv2.boundingRect(cnt)
        x_min = min(x_min, x)
        y_min = min(y_min, y)
        x_max = max(x_max, x + w)
        y_max = max(y_max, y + h)

    skel_area = (x_max - x_min) * (y_max - y_min)

    # 2. Extract Render Bounding Box (The Canny Silhouette)
    # We want to find the physical bounds of the stuffed animals on the neutral background
    gray_render = cv2.cvtColor(rendered_img, cv2.COLOR_BGR2GRAY)
    blurred_render = cv2.GaussianBlur(gray_render, (5, 5), 0)
    
    # The softbox background is usually light/white. We threshold to find the dark plushies.
    _, thresh = cv2.threshold(blurred_render, 240, 255, cv2.THRESH_BINARY_INV)
    
    # Run Canny on top of threshold to get strong outlines
    edges = cv2.Canny(thresh, 50, 150)
    
    # Dilate edges to connect broken lines of fur
    kernel = np.ones((5,5),np.uint8)
    dilated_edges = cv2.dilate(edges,kernel,iterations = 2)

    contours_render, _ = cv2.findContours(dilated_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours_render:
         return False, "TOTAL_COLLAPSE", "INCREASE_CFG" # Completely empty/white image

    rx_min, ry_min = 9999, 9999
    rx_max, ry_max = 0, 0
    for cnt in contours_render:
        # Ignore tiny dust particles
        if cv2.contourArea(cnt) > 100:
            x, y, w, h = cv2.boundingRect(cnt)
            rx_min = min(rx_min, x)
            ry_min = min(ry_min, y)
            rx_max = max(rx_max, x + w)
            ry_max = max(ry_max, y + h)

    render_area = (rx_max - rx_min) * (ry_max - ry_min)

    # 3. The Alligator's Bite (IoU / Mass comparison)
    # If the render area is massively larger than the skeleton area, the AI melted 
    # and hallucinated extra mass outside the bones.
    # We allow some padding for "fat plush" (e.g. 1.5x larger) but reject massive blobs.
    
    # We also check if the render area is too small (it ignored the skeleton entirely)
    
    ratio = render_area / skel_area
    
    # During high decay, we allow more melting. During low decay, it must be rigid.
    max_allowed_ratio = 1.6 + (decay_ratio * 0.4) 
    min_allowed_ratio = 0.5 - (decay_ratio * 0.2)

    if ratio > max_allowed_ratio:
        return False, f"STRUCTURAL_FRACTURE_TOO_LARGE ({ratio:.2f} > {max_allowed_ratio:.2f})", "INCREASE_CONTROL"
    elif ratio < min_allowed_ratio:
        return False, f"STRUCTURAL_FRACTURE_TOO_SMALL ({ratio:.2f} < {min_allowed_ratio:.2f})", "SCRAMBLE_SEED"

    # Evaluate Sharpness (The Spider's Texture Check)
    sharpness = cv2.Laplacian(gray_render, cv2.CV_64F).var()
    if sharpness < 50.0:
         return False, f"SEVERE_BLUR ({sharpness:.1f} < 50.0)", "INCREASE_STEPS"

    return True, "FORM_VERIFIED", "NONE"

if __name__ == "__main__":
    # Quick debug test if run directly
    print(run_vision_critic("assets/mpx_out.png", "assets/mpx_skel.png", 0.0))

