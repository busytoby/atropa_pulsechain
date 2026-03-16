import ctypes
import base64
import os
import json

def get_b64_from_raw(path, w=256, h=256):
    if not os.path.exists(path): return None
    with open(path, "rb") as f:
        raw_data = f.read()
    # Use silhouette_lib's JPEG encoder to get a valid image for the VLM
    # (Simulating what the broadcaster does)
    # For this script, we'll just use PIL to save a temporary JPEG
    from PIL import Image
    img = Image.frombytes("RGB", (w, h), raw_data)
    import io
    buf = io.BytesIO()
    img.save(buf, format="JPEG")
    return base64.b64encode(buf.getvalue()).decode('utf-8')

def main():
    silhouette_lib = ctypes.CDLL("./bin/libtsfi_silhouette.so")
    silhouette_lib.tsfi_ai_evaluate_vlm.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
    
    kernels = ["ultra", "pro", "bolt"]
    results = {}

    print("=== VLM Physical Compunction Audit ===")
    for k in kernels:
        path = f"tmp/bench_{k}.raw"
        b64 = get_b64_from_raw(path)
        if not b64:
            print(f" [SKIP] {k} (file missing)")
            continue
            
        vlm_resp = ctypes.create_string_buffer(512)
        q = b"Rate the photorealistic material quality and physical compunction of this teddy bear from 0 to 100. Answer with ONLY the number."
        
        silhouette_lib.tsfi_ai_evaluate_vlm(b64.encode('utf-8'), q, vlm_resp, 512)
        score = vlm_resp.value.decode('utf-8').strip()
        results[k] = score
        print(f" [{k.upper()}] VLM Score: {score}")

    with open("tmp/vlm_audit.json", "w") as f:
        json.dump(results, f)

if __name__ == "__main__":
    main()
