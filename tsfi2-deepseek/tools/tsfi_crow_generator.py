import os
import time
import subprocess
import random
from PIL import Image

def generate_crow(steps, seed):
    # Enforce high quality for live broadcast
    steps = 20
    
    base_prompt = "extreme macro photography of a stuffed animal crow plush, hyper-detailed individual synthetic feathers, matted black faux-plumage textures, visible stitching, soft plush under-down, studio lighting catching iridescent sheen, neutral background, 16k resolution masterpiece"
    output_raw = "tmp/crow_broadcast.raw"
    output_jpg = "/tmp/tsfi_broadcast.jpg"
    
    # Using bin/tsfi_sd_worker
    # Usage: <prompt> <output.raw> <use_shm> <profile> <steps> <method> [cfg]
    cmd = [
        "bin/tsfi_sd_worker",
        base_prompt,
        output_raw,
        "1", # use_shm ENABLED (Uses live skeleton from tsfi_crow_skeleton_writer.py)
        "sd15",
        str(steps),
        "euler_a",
        "7.5" # Strong CFG for feather definition
    ]
    
    print(f"[GENERATOR] Rendering Crow | Steps: {steps} | CFG: 7.5 | EXTRA FEATHER FOCUS")
    
    # Cycle through feather-specific detail enhancers
    feather_tags = [
        "individual synthetic feather barbs",
        "overlapping plush plumage scales",
        "matted black faux fur texture",
        "iridescent synthetic sheen",
        "hyper-detailed feather stitching",
        "soft downy synthetic undercoat"
    ]
    current_prompt = f"{base_prompt}, {random.choice(feather_tags)}"
    cmd[1] = current_prompt
    
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        print(f"[ERROR] SD Worker failed: {result.stderr.decode()}")
        return False
        
    if os.path.exists(output_raw):
        try:
            with open(output_raw, "rb") as f:
                raw_data = f.read()
            # SD Worker outputs 512x512 RGB raw
            expected_size = 512 * 512 * 3
            if len(raw_data) >= expected_size:
                img = Image.frombytes("RGB", (512, 512), raw_data[:expected_size])
                img.save(output_jpg, "JPEG", quality=95)
                print(f"[SUCCESS] Broadcast Updated: {output_jpg}")
                return True
            else:
                print(f"[ERROR] Output data too small: {len(raw_data)}")
                return False
        except Exception as e:
            print(f"[ERROR] Image processing failed: {e}")
            return False
    return False

if __name__ == "__main__":
    if not os.path.exists("tmp"):
        os.makedirs("tmp")
        
    print("=== TSFi Live Crow Broadcast Generator Active (FEATHER FOCUS) ===")
    print("-> Ensure tools/tsfi_crow_skeleton_writer.py is running for live ballet postures!")
    
    while True:
        generate_crow(20, random.randint(0, 1000000))
        # High quality renders take ~1-2s on RX 9070 XT, no need for long sleep
        time.sleep(0.5)
