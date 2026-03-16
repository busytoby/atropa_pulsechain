import os
import mmap
import struct
import torch
from transformers import AutoModelForCausalLM, AutoProcessor
from PIL import Image

def capture_wayland_framebuffer():
    fb_path = "/dev/shm/tsfi_wayland_fb"
    w = 800
    h = 600
    sz = w * h * 4
    
    if not os.path.exists(fb_path):
        print("[FRACTURE] Framebuffer offline.")
        return None
        
    fd = os.open(fb_path, os.O_RDONLY)
    fb_map = mmap.mmap(fd, sz, mmap.MAP_SHARED, mmap.PROT_READ)
    
    # We must convert the raw RGBA/ARGB bytearray into a PIL Image for the VLM.
    # We create an empty PIL Image and put the bytes in.
    # Wayland memfd is typically BGRA or ARGB depending on endianness.
    # Using 'RGBA' generic mapping to feed the vision tensor.
    raw_bytes = fb_map.read(sz)
    
    # Close memory mapping
    fb_map.close()
    os.close(fd)
    
    try:
        # Convert raw buffer to Image. Assume BGRA/RGBA.
        image = Image.frombytes("RGBA", (w, h), raw_bytes, "raw")
        # Convert to pure RGB to satisfy the DeepSeek Vision Encoder
        image = image.convert("RGB")
        return image
    except Exception as e:
        print(f"[FRACTURE] Failed to serialize physical pixels: {e}")
        return None

def run_optical_nerve():
    print("=== TSFi Moondream Optical Matrix ===")
    
    img = capture_wayland_framebuffer()
    if img is None:
        return
        
    print("[INFO] Physical Wayland buffer converted to Vision Tensor.")
    
    # Define Moondream architecture (Ultra-fast, natively supported)
    model_id = "vikhyatk/moondream2"
    revision = "2024-04-02"
    
    print(f"-> Booting Vision-Language Architect: {model_id}...", flush=True)
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            model_id, trust_remote_code=True, revision=revision
        ).to("cuda")
        processor = AutoProcessor.from_pretrained(
            model_id, trust_remote_code=True, revision=revision
        )
        
        # Format the multimodal input
        prompt = "Look at this physical Wayland screen buffer. Describe the precise geometry, colors, and shapes you see. Be incredibly brief and exact."
        
        print("\\n[ACTIVE] Neural Optical Scan Engaged...\\n")
        
        # Moondream custom API
        enc_image = model.encode_image(img)
        response = model.answer_question(enc_image, prompt, processor)
        
        print("=== MOONDREAM ARCHITECTURAL OBSERVATION ===")
        print(response)
        print("===========================================")
        
    except Exception as e:
        print(f"\\n[FRACTURE] Optical Network Failure: {e}")

if __name__ == "__main__":
    run_optical_nerve()
