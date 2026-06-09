# Sega Saturn VDP1 Quad-Based Rendering Architecture

The **Sega Saturn VDP1** is a specialized coprocessor responsible for drawing sprites, lines, and polygons. Unlike modern 3D graphics processors that rely on triangle rasterization, VDP1 is designed entirely around **Quadrilateral-Based Rendering**.

---

## 1. VDP1 Command Tables

VDP1 executes drawing operations by parsing a list of commands stored in VRAM. This list is composed of **Command Blocks (CMD)**.

### Command Block Layout (CMD)
Each CMD block is 32 bytes wide and contains the following fields:

| Offset (Bytes) | Name | Description |
| :---: | :--- | :--- |
| `0x00` | **CTRL** | Control word (Determines sprite type: Normal, Scaled, Distorted, or Polygon) |
| `0x02` | **PMOD** | Plot Mode (Color mode: 4bpp, 8bpp, 16bpp high color, Gouraud shading flags) |
| `0x04` | **COLR** | Color/Palette address |
| `0x06` | **SRCA** | Source Texture address in VRAM |
| `0x08` | **SIZE** | Texture width and height ($W \times H$) |
| `0x0A` | **XA, YA** | Vertex A Coordinate ($X_0, Y_0$) |
| `0x0E` | **XB, YB** | Vertex B Coordinate ($X_1, Y_1$) |
| `0x12` | **XC, YC** | Vertex C Coordinate ($X_2, Y_2$) |
| `0x16` | **XD, YD** | Vertex D Coordinate ($X_3, Y_3$) |
| `0x1A` | **GRDA** | Gouraud Shading Table address |

---

## 2. Quadrilateral vs. Triangle Rendering

Because VDP1 distorts textures to fit four arbitrary vertices (A, B, C, D), it maps textures using bilinear interpolation:

```
    A (X0, Y0) ────────────── B (X1, Y1)
       │                        │
       │     P(x,y)             │
       │    (Bilinear UV)       │
       │                        │
    D (X3, Y3) ────────────── C (X2, Y2)
```

During rasterization, the VDP1 spans horizontal lines between the edges $AD$ and $BC$, interpolating texture coordinates ($U, V$) along the way. If a quadrilateral is highly distorted or folded (concave), it can lead to visual artifacts (such as pixels missing or "texture tearing"), a signature characteristic of Sega Saturn 3D games.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_saturn_vdp1.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_saturn_vdp1.js).

This script:
1. Simulates a VDP1 VRAM command stream containing Distorted Sprites and Polygons.
2. Implements a bilinear interpolator that computes $X, Y$ coordinate maps and texture $U, V$ parameters across a distorted quadrilateral.
3. Validates vertex mappings and bounding box boundary checks.
