import math

def generate_sphere(center, radius, scale, u_segments=16, v_segments=16):
    vertices = []
    normals = []
    uvs = []
    faces = []
    
    cx, cy, cz = center
    sx, sy, sz = scale

    for i in range(u_segments + 1):
        theta = i * math.pi / u_segments
        sin_t = math.sin(theta)
        cos_t = math.cos(theta)
        
        for j in range(v_segments + 1):
            phi = j * 2.0 * math.pi / v_segments
            sin_p = math.sin(phi)
            cos_p = math.cos(phi)
            
            # Normal
            nx = sin_t * cos_p
            ny = cos_t
            nz = sin_t * sin_p
            
            # Vertex
            vx = cx + radius * nx * sx
            vy = cy + radius * ny * sy
            vz = cz + radius * nz * sz
            
            # UV
            u = j / v_segments
            v = i / u_segments
            
            vertices.append((vx, vy, vz))
            normals.append((nx, ny, nz))
            uvs.append((u, v))
            
    # Faces
    for i in range(u_segments):
        for j in range(v_segments):
            first = i * (v_segments + 1) + j
            second = first + v_segments + 1
            
            # Triangle 1
            faces.append((first, second, first + 1))
            # Triangle 2
            faces.append((second, second + 1, first + 1))
            
    return vertices, normals, uvs, faces

def main():
    parts = [
        # Center, Radius, Scale
        # Torso
        ((0.0, 0.0, 0.0), 1.0, (1.0, 1.2, 0.9)),
        # Head
        ((0.0, 1.2, 0.0), 0.7, (1.0, 0.95, 0.95)),
        # Snout
        ((0.0, 1.05, 0.5), 0.22, (1.1, 0.8, 1.0)),
        # Left Ear
        ((-0.5, 1.7, 0.1), 0.22, (1.0, 1.0, 0.5)),
        # Right Ear
        ((0.5, 1.7, 0.1), 0.22, (1.0, 1.0, 0.5)),
        # Left Arm
        ((-0.8, 0.3, 0.0), 0.28, (0.8, 1.4, 0.8)),
        # Right Arm
        ((0.8, 0.3, 0.0), 0.28, (0.8, 1.4, 0.8)),
        # Left Leg
        ((-0.5, -1.0, 0.2), 0.35, (0.9, 1.3, 0.9)),
        # Right Leg
        ((0.5, -1.0, 0.2), 0.35, (0.9, 1.3, 0.9)),
    ]
    
    all_vertices = []
    all_normals = []
    all_uvs = []
    all_faces = []
    
    v_offset = 1
    for center, radius, scale in parts:
        verts, norms, uvs, faces = generate_sphere(center, radius, scale)
        all_vertices.extend(verts)
        all_normals.extend(norms)
        all_uvs.extend(uvs)
        
        for f in faces:
            # Shift face indices by current vertex offset (1-based OBJ)
            all_faces.append((f[0] + v_offset, f[1] + v_offset, f[2] + v_offset))
        v_offset += len(verts)
        
    with open("assets/teddy_bear.obj", "w") as f:
        f.write("# Standard Teddy Bear Model (Generated)\n")
        for v in all_vertices:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
        for vn in all_normals:
            f.write(f"vn {vn[0]:.4f} {vn[1]:.4f} {vn[2]:.4f}\n")
        for vt in all_uvs:
            f.write(f"vt {vt[0]:.4f} {vt[1]:.4f}\n")
        for face in all_faces:
            # Output with vertex/uv/normal index format
            f.write(f"f {face[0]}/{face[0]}/{face[0]} {face[1]}/{face[1]}/{face[1]} {face[2]}/{face[2]}/{face[2]}\n")
            
    print(f"Generated teddy_bear.obj: {len(all_vertices)} vertices, {len(all_faces)} faces.")

if __name__ == "__main__":
    main()
