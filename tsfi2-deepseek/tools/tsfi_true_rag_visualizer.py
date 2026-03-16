import tkinter as tk
import json
import mmap
import ctypes
import os

# Physical Mapping of the Trilateral Telemetry
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

class TrueRagVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("TSFi RAG Visualizer: DeepSeek Autonomous Blueprint")
        self.w = 1024
        self.h = 1536
        self.root.geometry(f"{self.w}x{self.h}")
        self.root.configure(bg='black')
        self.root.bind("<Escape>", self.terminate)
        
        self.canvas = tk.Canvas(root, width=self.w, height=self.h, bg='black', highlightthickness=0)
        self.canvas.pack()
        
        # 1. Load the Authentic DeepSeek RAG Blueprint
        self.blueprint = []
        try:
            with open("assets/rag_fa_blueprint.json", "r") as f:
                self.blueprint = json.load(f)
            print("[INFO] DeepSeek RAG Blueprint Loaded.")
        except Exception as e:
            print(f"[FRACTURE] Failed to load JSON: {e}")
            
        # 2. Connect to Live Hardware Registry
        self.registry = None
        try:
            shm_fd = os.open("/dev/shm/tsfi_global_registry", os.O_RDWR)
            reg_map = mmap.mmap(shm_fd, ctypes.sizeof(TsfiGlobalRegistry), mmap.MAP_SHARED, mmap.PROT_READ)
            self.registry = TsfiGlobalRegistry.from_buffer(reg_map)
            print("[INFO] Live Telemetry Registry Connected.")
        except:
            print("[WARN] Registry offline. Running visualizer in simulated clock mode.")
            self.sim_cycles = 0
            
        self.animate_rag()
        
    def terminate(self, event=None):
        print("\\n[TERMINATE] Escaping physical window geometry.")
        self.root.destroy()
        
    def hex_to_rgb(self, hex_str):
        # Convert 0xFF00AAFF to tkinter #00AAFF
        try:
            val = int(hex_str, 16)
            r = (val >> 16) & 0xFF
            g = (val >> 8) & 0xFF
            b = val & 0xFF
            return f"#{r:02x}{g:02x}{b:02x}"
        except:
            return "#FFFFFF"

    def animate_rag(self):
        self.canvas.delete("all")
        
        # Poll the hardware entropy
        if self.registry:
            cycles = self.registry.firmware_cycle_count
        else:
            self.sim_cycles += 128
            cycles = self.sim_cycles
            
        # The Mathematical Constraint
        prime = 953473
        live_base = (cycles * 31) % prime
        live_signal = (cycles * 17) % prime
        live_channel = (live_base * live_signal) % prime # Simulating the Tune() modulus
        
        # Draw exactly what DeepSeek commanded in the JSON
        for obj in self.blueprint:
            x = obj.get("x", 0)
            y = obj.get("y", 0)
            base_w = obj.get("w", 100)
            h = obj.get("h", 50)
            raw_color = obj.get("color", "0xFFFFFFFF")
            tk_color = self.hex_to_rgb(raw_color)
            name = obj.get("object", "Unknown")
            
            # Draw exactly what DeepSeek commanded in the JSON
            # Dynamically mutate the geometry width or color based on the live physical variable value
            shape = obj.get("shape", "rect")
            if name == "base":
                w = int((live_base / prime) * 600) + 10
            elif name == "signal" or "eyes" in name:
                w = int((live_signal / prime) * 200) + 10
            elif name == "channel":
                w = int((live_channel / prime) * 800) + 10
                if live_channel > 800000:
                    tk_color = "#00FF00" 
                    
            if shape == "oval":
                self.canvas.create_oval(x, y, x + w, y + h, fill=tk_color, outline="#333333")
            else:
                self.canvas.create_rectangle(x, y, x + w, y + h, fill=tk_color, outline="#333333")
            
            # Overlay the actual Semantic Codebase knowledge (RAG Output)
            text_disp = f"var: {name}"
            val_disp = ""
            if name == "base": val_disp = str(live_base)
            if name == "signal": val_disp = str(live_signal)
            if name == "channel": val_disp = f"{live_channel} (modPow Lock)"
            
            self.canvas.create_text(x + 10, y + 15, text=text_disp, fill="white", anchor="w", font=("Courier", 10))
            self.canvas.create_text(x + 10, y + 35, text=val_disp, fill="white", anchor="w", font=("Courier", 12, "bold"))

        # 60fps loop
        self.root.after(16, self.animate_rag)

if __name__ == "__main__":
    print("=== TSFi Live RAG Execution ===")
    root = tk.Tk()
    app = TrueRagVisualizer(root)
    root.mainloop()
