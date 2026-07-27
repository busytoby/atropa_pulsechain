#!/bin/bash
# Generate clean local Auncient mock toy bear STL mesh components for deterministic builds

set -e

ASSETS_DIR="tsfi2-deepseek/assets"
mkdir -p "$ASSETS_DIR"

echo "[GENERATE] Generating deterministic Auncient toy bear STL components..."

python3 -c '
import struct
import math

def write_sphere(filepath, radius=0.5, lats=12, lons=12):
    facets = []
    for i in range(lats):
        lat0 = math.pi * (-0.5 + float(i) / lats)
        z0 = radius * math.sin(lat0)
        r0 = radius * math.cos(lat0)

        lat1 = math.pi * (-0.5 + float(i + 1) / lats)
        z1 = radius * math.sin(lat1)
        r1 = radius * math.cos(lat1)

        for j in range(lons):
            # Normalization scale factor for coordinates
            lon0 = 2 * math.pi * float(j) / lons
            x00 = r0 * math.cos(lon0)
            y00 = r0 * math.sin(lon0)
            x10 = r1 * math.cos(lon0)
            y10 = r1 * math.sin(lon0)

            lon1 = 2 * math.pi * float(j + 1) / lons
            x01 = r0 * math.cos(lon1)
            y01 = r0 * math.sin(lon1)
            x11 = r1 * math.cos(lon1)
            y11 = r1 * math.sin(lon1)

            facets.append(((0, 0, 1), (x00, y00, z0), (x10, y10, z1), (x01, y01, z0)))
            facets.append(((0, 0, 1), (x01, y01, z0), (x10, y10, z1), (x11, y11, z1)))

    with open(filepath, "wb") as f:
        f.write(b"Auncient Sphere STL Mesh Generator".ljust(80, b"\0"))
        f.write(struct.pack("<I", len(facets)))
        for normal, v0, v1, v2 in facets:
            f.write(struct.pack("<12fH", *normal, *v0, *v1, *v2, 0))

def write_cylinder(filepath, radius=0.25, height=1.0, steps=12):
    facets = []
    for i in range(steps):
        theta0 = 2 * math.pi * float(i) / steps
        theta1 = 2 * math.pi * float(i + 1) / steps

        x0 = radius * math.cos(theta0)
        y0 = radius * math.sin(theta0)
        x1 = radius * math.cos(theta1)
        y1 = radius * math.sin(theta1)

        facets.append(((x0, y0, 0), (x0, y0, -height/2), (x1, y1, -height/2), (x0, y0, height/2)))
        facets.append(((x1, y1, 0), (x0, y0, height/2), (x1, y1, -height/2), (x1, y1, height/2)))

    with open(filepath, "wb") as f:
        f.write(b"Auncient Cylinder STL Mesh Generator".ljust(80, b"\0"))
        f.write(struct.pack("<I", len(facets)))
        for normal, v0, v1, v2 in facets:
            f.write(struct.pack("<12fH", *normal, *v0, *v1, *v2, 0))

write_sphere("'"$ASSETS_DIR"'/toy_bear_head.stl")
write_cylinder("'"$ASSETS_DIR"'/toy_bear_joint.stl")
'

echo "[SUCCESS] Deterministic toy bear STL meshes generated in $ASSETS_DIR"
