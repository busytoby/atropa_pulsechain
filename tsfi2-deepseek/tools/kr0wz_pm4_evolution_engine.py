import sys
import os
import socket
import mmap
import struct
import json
import re
import time
from llama_cpp import Llama

def generate_pm4_scene():
    print("=== DeepSeek Autonomous PM4 Evolution Engine ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.", flush=True)
        return

    print("-> Loading Neural Matrix into VRAM...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=4096,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    # Establish the zero-copy PM4 execution stream for the Wayland C-binary
    pm4_shm_fd = os.open("/dev/shm/tsfi_pm4_stream", os.O_CREAT | os.O_RDWR, 0o666)
    os.ftruncate(pm4_shm_fd, 8192) # 8KB PM4 Stream Buffer
    pm4_map = mmap.mmap(pm4_shm_fd, 8192, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)

    UDP_IP = "127.0.0.1"
    UDP_PORT = 10044
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind((UDP_IP, UDP_PORT))
    except:
        pass
        
    print(f"[ACTIVE] Daemon shifting to Continuous Real-Time Evolution Mode...\\n", flush=True)
    
    trilateral_epoch = 0
    animation_step = 0
    
    while True:
        animation_step += 1
        
        # Advance the epoch to simulate the mechanical lifecycle (0 to 8)
        if animation_step % 5 == 0:
            trilateral_epoch = (trilateral_epoch + 1) % 9
            
        print(f"\\n[SCENE GENERATION] Evolving PM4 Matrix... (Frame {animation_step} | SEAL{trilateral_epoch})", flush=True)
        
        prompt = f"""You are the autonomous Art Director for the TSFi Wayland UI. 
You must visually animate the Trilateral Manifold onto an 800x600 screen using strict geometric primitives.
The current simulation state is: SEAL{trilateral_epoch}. 
* If SEAL0 (SHOOT): Draw the Rod and Cone far apart.
* If SEAL4 (CONJUGATE): Draw the Rod and Cone colliding in the center.
* If SEAL8 (MAGNETIZE): Draw them fused together with the Daiichi/Ichidai Reciprocity Shadow beneath them.
Generate a JSON array of rectangles to draw this exact state.
Example format:
[
  {{"object": "Rod", "x": 100, "y": 250, "w": 100, "h": 100, "color": "0xFF00AAFF"}},
  {{"object": "Cone", "x": 600, "y": 250, "w": 100, "h": 100, "color": "0xFF00AAFF"}}
]
Respond ONLY with the raw JSON array. No markdown ticks. No conversational text."""
        
        formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
        
        # Speed up the inference by reducing max tokens and adjusting temperature
        output = llm(formatted_prompt, max_tokens=256, temperature=0.5, stop=["### Instruction:"], echo=False)
        report = output['choices'][0]['text'].strip()
        
        # Extract JSON
        code_match = re.search(r'\[(.*?)\]', report.replace('\\n', ' '), re.DOTALL)
        if not code_match:
            print("[FRACTURE] AI failed to generate geometric JSON.")
            continue
            
        json_str = "[" + code_match.group(1) + "]"
        
        try:
            scene_objects = json.loads(json_str)
        except:
            print("[FRACTURE] Invalid JSON Geometry.")
            continue
            
        # Compile JSON into Physical AMD PM4 Command Packets
        PKT3_DRAW_RECT = 0x2A
        pm4_buffer = bytearray()
        packet_count = 0
        
        for obj in scene_objects:
            try:
                x = int(obj.get("x", 0))
                y = int(obj.get("y", 0))
                w = int(obj.get("w", 10))
                h = int(obj.get("h", 10))
                color = int(obj.get("color", "0xFFFFFFFF"), 16)
                
                # PM4 Packet Type 3 Header: (3 << 30) | (count << 16) | (opcode << 8)
                header = (3 << 30) | (5 << 16) | (PKT3_DRAW_RECT << 8)
                pm4_buffer.extend(struct.pack('=I', header))
                pm4_buffer.extend(struct.pack('=I', x))
                pm4_buffer.extend(struct.pack('=I', y))
                pm4_buffer.extend(struct.pack('=I', w))
                pm4_buffer.extend(struct.pack('=I', h))
                pm4_buffer.extend(struct.pack('=I', color))
                packet_count += 6
            except Exception as e:
                pass
                
        # Write the compiled PM4 stream directly into the zero-copy buffer
        pm4_map.seek(0)
        # Write total packet count as the first 4 bytes so C knows how much to read
        pm4_map.write(struct.pack('=I', packet_count))
        pm4_map.write(pm4_buffer)
        
        print(f"[PM4 SYNTHESIS] {packet_count} uint32_t packets injected into ReBAR buffer.")
        
        # Artificial biological delay to allow the Wayland compositor to breathe and render the frame
        time.sleep(3)

if __name__ == "__main__":
    generate_pm4_scene()