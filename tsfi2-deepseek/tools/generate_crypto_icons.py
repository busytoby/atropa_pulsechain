import os
import subprocess
from PIL import Image

# The 10 tokens discovered from the mariarahel LAU Topographical Traversal
tokens = [
    "mariarahel",
    "CHATLOG Void",
    "CHATLOG Siu",
    "CHATLOG Yang",
    "CHATLOG Yau",
    "CHATLOG Zhou",
    "CHATLOG Zheng",
    "CHATLOG Yi",
    "CHATLOG Shio",
    "Shio Rod"
]

output_dir = "assets/crypto_icons"
os.makedirs(output_dir, exist_ok=True)

W, H = 512, 512
base_prompt = "clean minimalist line art icon, elegant ink drawing, bold black outlines on pure white background, vector graphics style, sharp edges, no shading, high contrast, perfect logo design, cryptocurrency token symbol"

print(f"=== TSFi Crypto Line Art Batch Generator ===")
print(f"Targeting {len(tokens)} entities from the mariarahel LAU provenance chain.\n")

for i, token in enumerate(tokens):
    # Formulate the prompt
    prompt = f"a visual symbolic representation of '{token}', {base_prompt}"
    
    # Path for the raw uncompressed bytes from the C-worker
    out_raw = f"tmp/token_{i}.raw"
    
    # Path for the final compressed PNG image
    clean_name = token.replace(' ', '_').lower()
    out_png = os.path.join(output_dir, f"{i:02d}_{clean_name}.png")
    
    print(f"[{i+1}/{len(tokens)}] Synthesizing '{token}'...")
    
    cmd = [
        "bin/tsfi_sd_worker",
        prompt,
        out_raw,
        "0", # use_shm = false (we are generating from pure text here, no scribble)
        "sd15",
        "20", # steps
        "euler_a",
        "7.5" # cfg
    ]
    
    # We do not need the ControlNets here since we are doing pure text-to-image
    # generation for the base concepts.
    res = subprocess.run(cmd, capture_output=True)
    
    if res.returncode == 0 and os.path.exists(out_raw):
        with open(out_raw, "rb") as f:
            out_data = f.read()
            
        expected_sz = W * H * 3
        if len(out_data) >= expected_sz:
            img = Image.frombytes("RGB", (W, H), out_data[:expected_sz])
            img.save(out_png)
            print(f"  -> Saved to {out_png}")
            os.remove(out_raw)
        else:
            print(f"  -> [ERROR] Raw output size mismatch for {token}. Got {len(out_data)} bytes.")
    else:
        err = res.stderr.decode('utf-8').strip()
        print(f"  -> [ERROR] Generation failed for {token}. SD Worker stderr: {err}")

print("\n=== Batch Generation Complete ===")
