import os
import time
import mmap
import struct
from llama_cpp import Llama

STRUCT_FMT = '=fffii256s'
SHM_FILE = '/dev/shm/tsfi_acoustic_space'

def run_agent_listener():
    print("=== TSFi Multi-Agent Participant (DeepSeek Oracle) ===", flush=True)
    
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    if not os.path.exists(gguf_path):
        print("[FRACTURE] DeepSeek model missing.")
        return
        
    print("[INFO] Booting Neural Listener into Acoustic Space...", flush=True)
    llm = Llama(model_path=gguf_path, n_ctx=512, n_threads=8, n_gpu_layers=-1, verbose=False)
    
    # Wait for dysnomia simulator to create the registry
    while not os.path.exists(SHM_FILE):
        time.sleep(1)
        
    fd = os.open(SHM_FILE, os.O_RDWR)
    shm_map = mmap.mmap(fd, 276)
    
    last_pressure = 0.0
    
    while True:
        try:
            shm_map.seek(0)
            data = struct.unpack(STRUCT_FMT, shm_map.read(276))
            pressure, hz, static, flag, tgt, mono = data
            
            # If the economic pressure spikes dramatically, trigger the LLM!
            if pressure > 0.8 and last_pressure <= 0.8:
                print(f"\\n[ACOUSTIC SPIKE DETECTED] Heartbeat: {hz:.1f}Hz. Alerting DeepSeek...")
                
                prompt = f"""You are the Sovereign Oracle in the TSFi VTUBER broadcast.
The PulseChain Dysnomia economy is collapsing. The pDAI manifold pressure has spiked to {pressure:.2f}.
The background acoustic heartbeat is racing at {hz:.1f} Hz with {static*100:.0f}% Kr0wZ static.
Based on this terrifying acoustic pressure, how should the physical Teddy Bear react on stream?
Choose exactly one ID from this list:
0: Sitting upright
1: Slumped over in defeat
2: Standing tall in defiance
3: Curled up in a fetal position in panic

Respond with a single sentence explaining your choice, followed by the exact ID number (e.g., 'ID: 3')."""

                formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
                
                output = llm(formatted_prompt, max_tokens=100, temperature=0.7, stop=["### Instruction:"], echo=False)
                response = output['choices'][0]['text'].strip()
                
                # Parse the ID
                target_id = 1 # Default to slumped
                if "ID: 0" in response: target_id = 0
                elif "ID: 1" in response: target_id = 1
                elif "ID: 2" in response: target_id = 2
                elif "ID: 3" in response: target_id = 3
                
                print(f"  -> DeepSeek Logic: {response}")
                print(f"  -> Executing Behavioral Override (Genome ID {target_id})")
                
                # Write the override command directly back into the Acoustic Registry
                encoded_mono = response.encode('utf-8')[:255].ljust(256, b'\0')
                new_data = struct.pack(STRUCT_FMT, pressure, hz, static, 1, target_id, encoded_mono)
                shm_map.seek(0)
                shm_map.write(new_data)
                
                time.sleep(10) # Cooldown to prevent spamming
                
            last_pressure = pressure
            time.sleep(1)
            
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    run_agent_listener()
