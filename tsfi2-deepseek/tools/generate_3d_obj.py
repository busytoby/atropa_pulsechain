import cv2
import numpy as np
import sys
import os

def create_obj(rgb_path, depth_path, obj_path):
    print(f"[INFO] Compiling {rgb_path} and {depth_path} into 3D Mesh: {obj_path}")
    
    # 1. Load the images
    img = cv2.imread(rgb_path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    depth = cv2.imread(depth_path, cv2.IMREAD_GRAYSCALE)
    
    # 2. Resize to something manageable so the .obj file isn't massive
    target_size = 256
    img = cv2.resize(img, (target_size, target_size))
    depth = cv2.resize(depth, (target_size, target_size))
    
    h, w = img.shape[:2]
    
    # 3. Create the vertices
    vertices = []
    colors = []
    
    # The Genie noted the plushies are in front. 
    # We will use the depth map to push the pixels forward on the Z-axis.
    # A depth value of 255 (white) means closest to the camera.
    depth_scale = 0.5 # How deep the 3D extrusion should be
    
    for y in range(h):
        for x in range(w):
            z = depth[y, x] / 255.0 * depth_scale
            # We invert Y so the image isn't upside down in standard 3D viewers
            vertices.append(( (x/float(w)) - 0.5, 0.5 - (y/float(h)), z ))
            
            r, g, b = img[y, x]
            colors.append((r/255.0, g/255.0, b/255.0))
            
    # 4. Create the faces (triangulate the grid)
    faces = []
    for y in range(h - 1):
        for x in range(w - 1):
            # Calculate indices of the 4 corners of the current square
            idx1 = y * w + x + 1          # 1-based indexing for OBJ
            idx2 = y * w + (x + 1) + 1
            idx3 = (y + 1) * w + x + 1
            idx4 = (y + 1) * w + (x + 1) + 1
            
            # Simple threshold to prevent "rubber-banding". 
            # If the depth difference between two adjacent pixels is too large,
            # it means we hit an edge (like the side of the bear). We don't draw a face there.
            z1 = vertices[idx1-1][2]
            z2 = vertices[idx2-1][2]
            z3 = vertices[idx3-1][2]
            z4 = vertices[idx4-1][2]
            
            # If the depth jump is larger than 0.05, it's a silhouette edge. Cut the mesh.
            if abs(z1-z2) < 0.05 and abs(z1-z3) < 0.05 and abs(z1-z4) < 0.05:
                # Triangle 1
                faces.append((idx1, idx2, idx3))
                # Triangle 2
                faces.append((idx2, idx4, idx3))
                
    # 5. Write to .obj file
    with open(obj_path, 'w') as f:
        f.write("# TSFi Procedural 3D Mesh\n")
        f.write("# Generated from RGB-D via Genie Orchestration\n")
        
        # Write vertices and vertex colors
        for i in range(len(vertices)):
            v = vertices[i]
            c = colors[i]
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f} {c[0]:.4f} {c[1]:.4f} {c[2]:.4f}\n")
            
        # Write faces
        for face in faces:
            f.write(f"f {face[0]} {face[1]} {face[2]}\n")

    print(f"[SUCCESS] Wrote {len(vertices)} vertices and {len(faces)} faces to {obj_path}.")

if __name__ == "__main__":
    import glob
    depths = sorted(glob.glob("assets/depth_maps/depth_*.png"))
    rgbs = sorted(glob.glob("assets/depth_maps/rgb_*.png"))
    if depths and rgbs:
        create_obj(rgbs[-1], depths[-1], "assets/stuffed_animals_scene.obj")
    else:
        print("Error: No depth maps found.")
