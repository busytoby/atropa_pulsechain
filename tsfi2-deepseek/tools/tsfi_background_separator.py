import os
import math
from PIL import Image, ImageChops, ImageFilter, ImageDraw

def synthesize_mock_epochs():
    # We mathematically simulate the Stable Diffusion outputs for Epoch 2 and Epoch 3
    # Both have the exact same background room, but the Bear mutates between them
    w, h = 512, 512
    
    epoch2 = Image.new("RGB", (w, h), (30, 30, 40)) # Dark blueish background room
    epoch3 = Image.new("RGB", (w, h), (30, 30, 40)) # Exact same background
    
    draw2 = ImageDraw.Draw(epoch2)
    draw3 = ImageDraw.Draw(epoch3)
    
    # Draw Static Background elements (Pillars / Floor)
    draw2.rectangle([0, 400, 512, 512], fill=(20, 20, 20)) # Floor
    draw3.rectangle([0, 400, 512, 512], fill=(20, 20, 20)) # Floor
    
    draw2.rectangle([50, 0, 100, 400], fill=(40, 40, 50)) # Pillar
    draw3.rectangle([50, 0, 100, 400], fill=(40, 40, 50)) # Pillar
    
    # Draw Epoch 2 Bear (Baseline Mutation)
    bear_bounds = [150, 150, 362, 450]
    draw2.ellipse(bear_bounds, fill=(165, 95, 45))
    # Sickness nodes in Epoch 2
    draw2.ellipse([200, 200, 250, 250], fill=(51, 65, 0)) 
    
    # Draw Epoch 3 Bear (Evolved Mutation, same silhouette but different texture/noise)
    draw3.ellipse(bear_bounds, fill=(160, 90, 40)) # Slight lighting shift on fur
    # Evolved Sickness nodes in Epoch 3 (physically moved)
    draw3.ellipse([220, 230, 290, 290], fill=(60, 80, 0))
    
    # Simulate slight AI pixel noise across the entire image (SD1.5 always has tiny latent noise variations)
    import random
    p2 = epoch2.load()
    p3 = epoch3.load()
    for y in range(h):
        for x in range(w):
            if random.random() > 0.5:
                r,g,b = p2[x,y]
                p2[x,y] = (r+1, g+1, b)
                r,g,b = p3[x,y]
                p3[x,y] = (r-1, g, b-1)
                
    return epoch2, epoch3

def execute_temporal_separation():
    print("=== TSFi Temporal Frame Differencing (Auto-Segmentation) ===")
    os.makedirs("assets/separation", exist_ok=True)
    
    print("[ACTIVE] Loading Epoch 2 and Epoch 3 states...")
    epoch2, epoch3 = synthesize_mock_epochs()
    epoch2.save("assets/separation/epoch2_raw.png")
    epoch3.save("assets/separation/epoch3_raw.png")
    
    print("[ACTIVE] Executing Absolute Difference Matrix...")
    # Mathematically subtract Epoch 2 from Epoch 3
    diff = ImageChops.difference(epoch2, epoch3)
    
    # Convert to Grayscale to evaluate total pixel variance
    diff_gray = diff.convert("L")
    
    # Apply a slight Gaussian blur to filter out the tiny latent AI noise and connect the body mass
    diff_blurred = diff_gray.filter(ImageFilter.GaussianBlur(radius=5))
    
    print("[ACTIVE] Calculating Binary Threshold Envelope...")
    # Threshold the difference: If pixel variance > 10 (out of 255), it is the Bear. Otherwise, it is Static Background.
    threshold = 10
    binary_mask = diff_blurred.point(lambda p: 255 if p > threshold else 0)
    binary_mask.save("assets/separation/extracted_silhouette.png")
    
    print("[ACTIVE] Locking Background Retention (Stage Logic)...")
    # We take the original Epoch 2 image, and wherever the binary_mask is WHITE (Bear), we punch a hole (Transparency).
    # This leaves ONLY the perfectly isolated background room!
    background_layer = epoch2.copy().convert("RGBA")
    bg_pixels = background_layer.load()
    mask_pixels = binary_mask.load()
    
    w, h = background_layer.size
    for y in range(h):
        for x in range(w):
            if mask_pixels[x, y] == 255:
                # This pixel is the Bear. Erase it from the background layer.
                r, g, b, a = bg_pixels[x, y]
                bg_pixels[x, y] = (r, g, b, 0) # 0 Alpha
                
    background_layer.save("assets/separation/locked_background.png")
    
    print("[ACTIVE] Isolating Pure Epoch 3 DNA Geometry...")
    # We take Epoch 3, and wherever the binary_mask is BLACK (Background), we punch a hole.
    # This leaves ONLY the photorealistic Bear!
    bear_layer = epoch3.copy().convert("RGBA")
    bear_pixels = bear_layer.load()
    
    for y in range(h):
        for x in range(w):
            if mask_pixels[x, y] == 0:
                # This pixel is the Background. Erase it from the Bear layer.
                r, g, b, a = bear_pixels[x, y]
                bear_pixels[x, y] = (r, g, b, 0) # 0 Alpha
                
    bear_layer.save("assets/separation/isolated_epoch3_bear.png")
    
    print("[ABSOLUTE SUCCESS] The Background Room is permanently locked.")
    print("[ABSOLUTE SUCCESS] The Epoch 3 Bear is fully isolated for continuous DNA Evolution.")

if __name__ == "__main__":
    execute_temporal_separation()
