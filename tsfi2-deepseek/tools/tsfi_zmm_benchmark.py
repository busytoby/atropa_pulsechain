import time
import ctypes
import math
from PIL import Image
import io

def bench_pure_python(W=512, H=512):
    img = Image.new("RGB", (W, H), (0, 0, 0))
    pixels = img.load()
    pulse = math.sin(0.1) 
    for y in range(H):
        py = y / H
        if py < 0.1 or py > 0.95: continue
        for x in range(W):
            px = x / W
            if px < 0.1 or px > 0.9: continue
            
            dist_left = math.hypot(px - 0.400, py - 0.780)
            dist_right = math.hypot(px - 0.624, py - 0.780)
            k = 50.0
            shadow_val = math.exp(-k * dist_left) + math.exp(-k * dist_right)
            shadow_dist = -math.log(shadow_val) / k if shadow_val > 0 else 1.0
            
            if shadow_dist < 0.08:
                pixels[x, y] = (19, 19, 19)
    return img

def bench_avx512_zmm(W=512, H=512):
    # Load the TSFi AVX-512 Shared Object
    tsfi_lib = ctypes.CDLL("./bin/libtsfi_zmm.so")
    
    # Define the C-function signature
    # void generate_zmm_mask(uint8_t* pixels, int w, int h, int frame, int total_frames)
    tsfi_lib.generate_zmm_mask.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
    
    # Pre-allocate a contiguous memory block in Python
    buffer_size = W * H * 3
    pixel_buffer = (ctypes.c_uint8 * buffer_size)()
    
    # Blast the memory directly to the C-Kernel ZMM registers
    tsfi_lib.generate_zmm_mask(pixel_buffer, W, H, 10, 100)
    
    # Instantly convert back to an Image (Zero-Copy conceptually)
    return Image.frombuffer("RGB", (W, H), pixel_buffer, "raw", "RGB", 0, 1)

if __name__ == "__main__":
    print("=== TSFi Python AVX-512 / ZMM Benchmark ===")
    
    iterations = 50
    
    # 1. Benchmark standard Python scalar logic
    start = time.time()
    for _ in range(iterations):
        bench_pure_python(512, 512)
    py_time = (time.time() - start) / iterations
    
    # 2. Benchmark TSFi AVX-512 ZMM logic
    start = time.time()
    for _ in range(iterations):
        bench_avx512_zmm(512, 512)
    zmm_time = (time.time() - start) / iterations
    
    print(f"\\n[SCALAR] Pure Python Generation Time: {py_time * 1000:.2f} ms")
    print(f"[VECTOR] TSFi AVX-512 ZMM Generation Time: {zmm_time * 1000:.2f} ms")
    print(f"-> ZMM Speedup Factor: {py_time / zmm_time:.2f}x")
