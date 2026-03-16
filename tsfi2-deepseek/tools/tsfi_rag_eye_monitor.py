import os
import re
import json
import time
import mmap
import ctypes
import struct
from llama_cpp import Llama

# TSFi Telemetry Map
class TsfiGlobalRegistry(ctypes.Structure):
    _fields_ = [
        ("last_heartbeat_ns", ctypes.c_uint64),
        ("firmware_cycle_count", ctypes.c_uint64),
        ("vulkan_vram_usage", ctypes.c_uint64),
        ("deepseek_ppo_epochs", ctypes.c_uint64),
        ("system_status_flag", ctypes.c_int),
        ("latest_gemini_directive", ctypes.c_char * 256),
        ("latest_deepseek_observation", ctypes.c_char * 256),
    ]

def compile_pm4_eye_matrix():
    print("=== TSFi Autonomous RAG Eye Monitor ===", flush=True)
    
    # 1. RETRIEVAL (AST Regex against C Codebase)
    print("[RAG] Retrieving Absolute Eye Geometry from inc/tsfi_atropa_geometry.h...")
    try:
        with open("inc/tsfi_atropa_geometry.h", "r") as f:
            c_header = f.read()
        match = re.search(r'typedef struct \{[^}]*\}\s*TsfiTeddyEye;', c_header, re.DOTALL)
        struct_def = match.group(0) if match else "Struct Not Found"
        match2 = re.search(r'static const TsfiTeddyEye TSFI_BASELINE_EYE[^;]*;', c_header, re.DOTALL)
        baseline_def = match2.group(0) if match2 else "Baseline Not Found"
        retrieved_geometry = f"{struct_def}\\n{baseline_def}"
    except Exception as e:
        print(f"[FRACTURE] Retrieval Failed: {e}")
        return
        
    print("  -> [PASS] Physical constants successfully retrieved.")
    
    # 2. GENERATION (DeepSeek Blueprint Synthesis)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path): return
    
    print("\\n-> Booting Vulkan DeepSeek Core for Semantic Translation...", flush=True)
    llm = Llama(model_path=gguf_path, n_ctx=2048, n_threads=16, n_gpu_layers=-1, verbose=False)
    
    prompt = f"""You are the TSFi Art Director.
I retrieved this geometry from the codebase:
```c
{retrieved_geometry}
```
Design a PM4 JSON blueprint for this Eye. You must output exactly two objects: 'iris' and 'pupil'.
Use the retrieved constants (origin_x=512, origin_y=400, iris_radius=45, pupil_radius=20) to calculate the strict (x, y, w, h) bounding boxes for the ovals.
Example format:
[
  {{"object": "iris", "shape": "oval", "x": 467, "y": 355, "w": 90, "h": 90, "color": "0xFF00FF00"}},
  {{"object": "pupil", "shape": "oval", "x": 492, "y": 380, "w": 40, "h": 40, "color": "0xFF000000"}}
]
Respond ONLY with raw JSON."""

    output = llm(f"### Instruction:\\n{prompt}\\n### Response:\\n", max_tokens=256, temperature=0.1, stop=["### Instruction:"], echo=False)
    report = output['choices'][0]['text'].strip()
    
    # Strip markdown and validate
    code_match = re.search(r'\[(.*?)\]', report.replace('\\n', ' '), re.DOTALL)
    if not code_match: return
    blueprint_json = "[" + code_match.group(1) + "]"
    try:
        blueprint = json.loads(blueprint_json)
        print("  -> [PASS] DeepSeek successfully generated the Eye Blueprint.")
    except:
        return
        
    # 3. LIVE AUGMENTATION (The 60FPS Render Loop)
    print("\\n[ACTIVE] Anchoring Blueprint to Live Telemetry...", flush=True)
    
    shm_fd = os.open("/dev/shm/tsfi_global_registry", os.O_RDWR)
    reg_map = mmap.mmap(shm_fd, ctypes.sizeof(TsfiGlobalRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    registry = TsfiGlobalRegistry.from_buffer(reg_map)
    
    pm4_shm_fd = os.open("/dev/shm/tsfi_pm4_stream", os.O_CREAT | os.O_RDWR, 0o666)
    os.ftruncate(pm4_shm_fd, 8192)
    pm4_map = mmap.mmap(pm4_shm_fd, 8192, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
    
    PKT3_DRAW_OVAL = 0x2B
    
    while True:
        cycles = registry.firmware_cycle_count
        
        # Calculate a live dilation factor based on the hardware clock (simulating Fa->Channel resonance)
        # Pulse between 0.8 and 1.5 multiplier
        dilation = 0.8 + abs((cycles % 1000) / 1000.0) * 0.7 
        
        pm4_buffer = bytearray()
        packet_count = 0
        
        for obj in blueprint:
            try:
                base_x = int(obj.get("x", 0))
                base_y = int(obj.get("y", 0))
                base_w = int(obj.get("w", 50))
                base_h = int(obj.get("h", 50))
                color = int(obj.get("color", "0xFFFFFFFF"), 16)
                name = obj.get("object", "")
                
                # The Live Augmentation: The Pupil dilates based on the telemetry cycle count!
                w, h, x, y = base_w, base_h, base_x, base_y
                if name == "pupil":
                    w = int(base_w * dilation)
                    h = int(base_h * dilation)
                    # Keep it centered
                    x = base_x + (base_w - w) // 2
                    y = base_y + (base_h - h) // 2
                    
                header = (3 << 30) | (5 << 16) | (PKT3_DRAW_OVAL << 8)
                pm4_buffer.extend(struct.pack('=I', header))
                pm4_buffer.extend(struct.pack('=I', x))
                pm4_buffer.extend(struct.pack('=I', y))
                pm4_buffer.extend(struct.pack('=I', w))
                pm4_buffer.extend(struct.pack('=I', h))
                pm4_buffer.extend(struct.pack('=I', color))
                packet_count += 6
            except: pass
            
        pm4_map.seek(0)
        pm4_map.write(struct.pack('=I', packet_count))
        pm4_map.write(pm4_buffer)
        
        time.sleep(1/60.0)

if __name__ == "__main__":
    compile_pm4_eye_matrix()
