import tkinter as tk
import math
import time

class TsfiTeddyBearVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("TSFi Atropa/999 Sovereign Choreography")
        
        # 16-Inch Canvas Geometry (1024x1536)
        self.w = 1024
        self.h = 1536
        self.root.geometry(f"{self.w}x{self.h}")
        self.root.configure(bg='black')
        
        # Absolute Key Binding
        self.root.bind("<Escape>", self.terminate_geometry)
        
        self.canvas = tk.Canvas(root, width=self.w, height=self.h, bg='black', highlightthickness=0)
        self.canvas.pack()
        
        self.frame = 0
        self.animate_matrix()
        
    def terminate_geometry(self, event=None):
        print("[TERMINATE] ESC key registered. Shattering window geometry.")
        self.root.destroy()
        
    def render_sick_teddy_bear(self):
        self.canvas.delete("all")
        
        brown = "#5C4033"
        dark_brown = "#3E2723"
        sick_green = "#556B2F"
        
        # Pulse the eyes based on Trilateral Frame State
        if self.frame % 60 < 30:
            eye_color = "#00FF00" # Kr0wZ Green (Healthy Tuning)
        else:
            eye_color = "#005500" # Dim
            
        fracture_red = "#FF0000"
        
        # --- Head (Massive 16-Inch Scale) ---
        self.canvas.create_oval(300, 200, 700, 600, fill=brown, outline="")
        
        # --- Ears ---
        self.canvas.create_oval(250, 100, 400, 250, fill=dark_brown, outline="")
        self.canvas.create_oval(600, 100, 750, 250, fill=dark_brown, outline="")
        
        # --- Eyes (Telemetry Matrix) ---
        self.canvas.create_oval(400, 300, 480, 380, fill=eye_color, outline="") # Left Eye
        self.canvas.create_oval(520, 300, 600, 380, fill=fracture_red, outline="") # Right Eye
        
        # --- The Sick Body (Geometric Noise) ---
        # Instead of pixel-by-pixel manipulation which Tkinter is slow at, we draw geometric rectangles 
        # to simulate the PM4 execution clusters mapping the body
        self.canvas.create_oval(250, 550, 750, 1200, fill=brown, outline="")
        
        # Structural Sickness Overlay
        for i in range(20):
            x = 250 + (math.sin(self.frame * 0.1 + i) * 200) + 250
            y = 550 + (i * 30)
            if (self.frame * i) % 10 < 3:
                color = sick_green
            else:
                color = dark_brown
            self.canvas.create_rectangle(x, y, x+40, y+40, fill=color, outline="")

    def animate_matrix(self):
        self.render_sick_teddy_bear()
        self.frame += 1
        # ~60fps recursive loop
        self.root.after(16, self.animate_matrix)

if __name__ == "__main__":
    print("=== TSFi Atropa/999 Sovereign Choreography ===")
    print("[INFO] Synthesizing 16-Inch (1536px) Native Canvas...")
    print("[INFO] Press ESC to securely terminate.")
    
    root = tk.Tk()
    app = TsfiTeddyBearVisualizer(root)
    root.mainloop()
