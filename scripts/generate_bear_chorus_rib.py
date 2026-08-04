#!/usr/bin/env python3
import os
import math

def generate_rib_sequence():
    output_dir = "rib_sequence"
    os.makedirs(output_dir, exist_ok=True)
    
    # 5 Bear profiles (names and spatial configurations)
    bears = [
        {"name": "Trusty",  "x": -2.0, "y": 0.0, "z": 5.0, "color": [0.2, 0.6, 1.0]},
        {"name": "Aggro",   "x": 2.0,  "y": 0.0, "z": 5.0, "color": [1.0, 0.2, 0.2]},
        {"name": "Skeptic", "x": -1.0, "y": 1.5, "z": 6.0, "color": [0.8, 0.8, 0.2]},
        {"name": "Eerie",   "x": 1.0,  "y": 1.5, "z": 6.0, "color": [0.6, 0.2, 0.8]},
        {"name": "Coop",    "x": 0.0,  "y": -1.5, "z": 4.5, "color": [0.2, 0.8, 0.4]}
    ]
    
    # Generate 24 frames of RenderMan RIB sequence matching Hudson frame rate
    for frame in range(1, 25):
        rib_path = os.path.join(output_dir, f"frame_{frame:04d}.rib")
        with open(rib_path, "w") as f:
            f.write("##RenderMan RIB-Structure 1.0\n")
            f.write(f"FrameBegin {frame}\n")
            f.write('Display "frame_{:04d}.tiff" "file" "rgba"\n'.format(frame))
            f.write("Format 640 480 1.0\n")
            f.write("Projection \"perspective\" \"fov\" [45]\n")
            f.write("Translate 0 0 0\n")
            f.write("WorldBegin\n")
            
            # Position light source
            f.write("LightSource \"distantlight\" 1 \"intensity\" [1.5] \"from\" [0 5 -5] \"to\" [0 0 5]\n")
            
            # Draw each bear sphere modulated by frame steps
            for i, bear in enumerate(bears):
                # Apply dynamic Verlet vertex displacement math scale sync (Rule 14)
                displacement = 0.1 * math.sin(frame * 0.25 + i)
                radius = 0.8 + displacement
                
                f.write("AttributeBegin\n")
                f.write('Surface "plastic" "Kd" [.8] "Ks" [.2] "roughness" [.1] "specularcolor" [1 1 1]\n')
                f.write('Color [{:.2f} {:.2f} {:.2f}]\n'.format(bear["color"][0], bear["color"][1], bear["color"][2]))
                f.write('Translate {:.2f} {:.2f} {:.2f}\n'.format(bear["x"], bear["y"] + displacement, bear["z"]))
                f.write(f"Sphere {radius:.4f} -{radius:.4f} {radius:.4f} 360\n")
                f.write("AttributeEnd\n")
                
            f.write("WorldEnd\n")
            f.write("FrameEnd\n")
            
    print(f"Generated RenderMan RIB sequence in '{output_dir}/'")

if __name__ == "__main__":
    generate_rib_sequence()
