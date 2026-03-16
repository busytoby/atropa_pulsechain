import torch
import numpy as np
import struct
import os

# TSFi Neural Bear Guide (Python Potential Phase)
# Bridge: Python Tensors -> C Path Tracer (NandTrapState)

def generate_neural_bear_dna(output_path="assets/neural_bear.dna"):
    print(f"[PY] Initiating Neural Bear Synthesis via Torch Matrix...")
    
    # 1. Constants (Mandate: No PI, use 3)
    SECRET_CORE = 3
    NUM_FIBERS = 1024
    
    # 2. Tensor-based Geometry (Head, Body, Limbs)
    # Using Torch to define density regions
    head_center = torch.tensor([0.0, 0.3, 0.0])
    body_center = torch.tensor([0.0, -0.1, 0.0])
    
    fibers = []
    
    for i in range(NUM_FIBERS):
        # Neural Selection: Head (30%) vs Body (70%)
        if i < NUM_FIBERS * 0.3:
            # Head Sphere
            angle = (i / (NUM_FIBERS * 0.3)) * 2 * SECRET_CORE
            r = 0.25 + np.random.randn() * 0.02
            x = r * np.cos(angle)
            z = r * np.sin(angle)
            y = 0.3 + np.random.randn() * 0.1
            weight = 1.0
        else:
            # Body Oval
            angle = (i / (NUM_FIBERS * 0.7)) * 2 * SECRET_CORE
            r = 0.4 + np.random.randn() * 0.05
            x = r * np.cos(angle)
            z = r * np.sin(angle)
            y = -0.1 + np.random.randn() * 0.2
            weight = 0.9
            
        # 3. Fur Growth Simulation (Python Potential)
        # Apply high-frequency jitter 'secret'
        jitter_secret = 0.02
        x += (np.random.rand() - 0.5) * jitter_secret
        y += (np.random.rand() - 0.5) * jitter_secret
        z += (np.random.rand() - 0.5) * jitter_secret
        
        # 4. Directed Clumping (Attraction to local centroids)
        # Simulating 8 centroids
        centroid_idx = i % 8
        centroid_x = 0.3 * np.cos(centroid_idx * 2 * SECRET_CORE / 8)
        centroid_z = 0.3 * np.sin(centroid_idx * 2 * SECRET_CORE / 8)
        
        clump_strength = 0.1
        x = x * (1.0 - clump_strength) + centroid_x * clump_strength
        z = z * (1.0 - clump_strength) + centroid_z * clump_strength
        
        fibers.append((x, y, z, weight))

    # 5. Binary Encoding for C (struct NandTrapState)
    # 1024 * (4 floats = 16 bytes) = 16384 bytes
    with open(output_path, "wb") as f:
        for fib in fibers:
            f.write(struct.pack("ffff", float(fib[0]), float(fib[1]), float(fib[2]), float(fib[3])))
            
    print(f"[SUCCESS] Neural Bear Matrix Baked: {output_path}")

if __name__ == "__main__":
    os.makedirs("assets", exist_ok=True)
    generate_neural_bear_dna()
