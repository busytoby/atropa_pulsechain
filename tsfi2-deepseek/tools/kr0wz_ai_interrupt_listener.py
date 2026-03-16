import sys
import os
import socket
import mmap
import ctypes
import time
from llama_cpp import Llama

# Define the exact Python struct to map to the C physical struct
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

def listen_for_interrupts():
    print("=== DeepSeek Real-Time Interrupt Daemon ===", flush=True)
    gguf_path = "assets/DeepSeek-Coder-6.7B.gguf"
    
    if not os.path.exists(gguf_path):
        print("[FRACTURE] Model not found.", flush=True)
        return

    print("-> Loading Neural Matrix into VRAM (Persistent)...", flush=True)
    llm = Llama(
        model_path=gguf_path,
        n_ctx=2048,
        n_threads=16,
        n_gpu_layers=-1,
        verbose=False
    )
    
    # Open the Zero-Copy Registry
    try:
        shm_fd = os.open("/dev/shm/tsfi_global_registry", os.O_RDWR)
        shm_map = mmap.mmap(shm_fd, ctypes.sizeof(TsfiGlobalRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
        registry = TsfiGlobalRegistry.from_buffer(shm_map)
    except FileNotFoundError:
        print("[FAIL] Cannot map physical registry. Start broker first.")
        return

    # Bind UDP Socket
    UDP_IP = "127.0.0.1"
    UDP_PORT = 10044
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    
    print(f"[ACTIVE] Daemon listening for UDP Hardware Interrupts on Port {UDP_PORT}...\\n", flush=True)
    
    while True:
        data, addr = sock.recvfrom(1024) # Blocking call. Zero CPU usage until interrupt.
        print(f"\\n[INTERRUPT RECEIVED] Signal: {data.decode('utf-8')}", flush=True)
        
        # 1. Read Live Telemetry (Zero-Copy)
        flag = registry.system_status_flag
        cycles = registry.firmware_cycle_count
        
        # 2. Formulate Observation
        prompt = f"The TSFi physical telemetry just triggered an interrupt. The Firmware Cycle Count is {cycles}. The System Status Flag is {flag} (0=Healthy, 1=Spinlock, 2=Fracture). Provide a one-sentence architectural observation of this exact millisecond."
        formatted_prompt = f"### Instruction:\\n{prompt}\\n### Response:\\n"
        
        print("-> Generating Neural Observation...", flush=True)
        output = llm(
            formatted_prompt,
            max_tokens=64,
            stop=["### Instruction:"],
            echo=False
        )
        
        obs = output['choices'][0]['text'].strip().replace("\\n", " ")[:255]
        
        # 3. Inject Observation into Memory
        registry.latest_deepseek_observation = obs.encode('utf-8')
        
        # 4. Clear the flag
        registry.system_status_flag = 0
        
        print(f"[OBSERVATION LOGGED] {obs}")

if __name__ == "__main__":
    listen_for_interrupts()
