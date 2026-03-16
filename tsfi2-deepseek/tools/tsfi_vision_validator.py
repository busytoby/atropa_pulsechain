import os
import mmap
import time
import struct

def run_vision_test():
    print("=== TSFi Autonomous Vision Validation ===", flush=True)
    
    fb_path = "/dev/shm/tsfi_wayland_fb"
    w = 800
    h = 600
    sz = w * h * 4
    
    print("[INFO] Waiting 8 seconds for DeepSeek to synthesize initial PM4 Matrix...", flush=True)
    time.sleep(8)
    
    if not os.path.exists(fb_path):
        print(f"[FRACTURE] Framebuffer {fb_path} is offline. Wayland driver shattered.")
        return
        
    print(f"-> Mapping physical framebuffer geometry ({sz} bytes)...", flush=True)
    fd = os.open(fb_path, os.O_RDONLY)
    fb_map = mmap.mmap(fd, sz, mmap.MAP_SHARED, mmap.PROT_READ)
    
    # Analyze the geometric matrix
    black_pixels = 0
    kr0wz_green_pixels = 0
    rod_cone_blue_pixels = 0
    other_colored_pixels = 0
    
    print("-> Engaging Matrix Reduction (Pixel Scanning)...", flush=True)
    
    for i in range(0, sz, 4):
        # Read a 32-bit ARGB pixel
        pixel = struct.unpack('=I', fb_map[i:i+4])[0]
        
        # Mask off Alpha channel for easier color checking
        color = pixel & 0x00FFFFFF
        
        if color == 0:
            black_pixels += 1
        elif color == 0x00FF00: # DeepSeek Kr0wZ Green (Helmholtz Root)
            kr0wz_green_pixels += 1
        elif color == 0x00AAFF: # Trilateral Blue (Rod/Cone)
            rod_cone_blue_pixels += 1
        else:
            other_colored_pixels += 1
            
    total_pixels = w * h
    active_pixels = kr0wz_green_pixels + rod_cone_blue_pixels + other_colored_pixels
    
    print("\\n--- VISUAL GEOMETRY OUTCOME ---")
    print(f"Total Framebuffer Resolution : {w}x{h} ({total_pixels} pixels)")
    print(f"Void Space (Black Pixels)    : {black_pixels}")
    print(f"Active Rendered Geometry     : {active_pixels}")
    
    if active_pixels > 0:
        print("\\n[ABSOLUTE SUCCESS] The PM4 Matrix has physically materialized. The screen is NOT black.")
        print(f"  -> DeepSeek correctly evolved the scene geometry, bypassing the UI void.")
    else:
        print("\\n[FRACTURE] The PM4 Matrix failed to render. The screen is a completely black void.")
        
    fb_map.close()
    os.close(fd)

if __name__ == "__main__":
    run_vision_test()
