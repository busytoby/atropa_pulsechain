import sys
import ctypes
import mmap
import os
import time

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

def access_registry(command, payload=""):
    shm_fd = None
    try:
        shm_fd = os.open("/dev/shm/tsfi_global_registry", os.O_RDWR)
        shm_map = mmap.mmap(shm_fd, ctypes.sizeof(TsfiGlobalRegistry), mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE)
        
        # Cast the raw bytes directly into the C-struct
        registry = TsfiGlobalRegistry.from_buffer(shm_map)
        
        if command == "read":
            print("=== TSFi Trilateral Registry State ===")
            print(f"Firmware Cycle Count: {registry.firmware_cycle_count}")
            print(f"System Status Flag: {registry.system_status_flag}")
            print(f"Gemini Directive: {registry.latest_gemini_directive.decode('utf-8').strip()}")
            print(f"DeepSeek Observation: {registry.latest_deepseek_observation.decode('utf-8').strip()}")
        elif command == "write_gemini":
            registry.latest_gemini_directive = payload.encode('utf-8')
            print(f"[GEMINI INJECT] Wrote directive to Zero-Copy Registry.")
        elif command == "write_deepseek":
            registry.latest_deepseek_observation = payload.encode('utf-8')
            print(f"[DEEPSEEK INJECT] Wrote observation to Zero-Copy Registry.")
            
    except FileNotFoundError:
        print("[FRACTURE] Registry Broker is offline. /dev/shm/tsfi_global_registry not found.")
    finally:
        if shm_fd:
            os.close(shm_fd)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 kr0wz_registry_bridge.py [read|write_gemini|write_deepseek] <payload>")
        sys.exit(1)
        
    cmd = sys.argv[1]
    pay = sys.argv[2] if len(sys.argv) > 2 else ""
    access_registry(cmd, pay)
