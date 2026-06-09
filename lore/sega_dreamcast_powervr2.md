# Sega Dreamcast PowerVR2 (CLX2) TBDR Architecture

The **Sega Dreamcast** graphics pipeline is powered by the **NEC PowerVR2 (CLX2)** GPU. Rather than using standard immediate-mode rendering, the PowerVR2 pioneered **Tile-Based Deferred Rendering (TBDR)**, which dramatically optimizes memory bandwidth and resolves overdraw constraints.

---

## 1. The TBDR Pipeline

```
               [ 3D Scene Geometry ]
                         │
                         ▼
        ┌───────────────────────────────────┐
        │  1. Tile Accelerator (Binning)    │ (Group polygons into 32x32 tiles)
        └───────────────────────────────────┘
                         │
                         ▼
        ┌───────────────────────────────────┐
        │  2. ISP (Image Synthesis Proc)    │ (Deferred Depth Test & Z-Sort)
        └───────────────────────────────────┘
                         │
                         ▼
        ┌───────────────────────────────────┐
        │  3. TSP (Texture/Shading Proc)    │ (Shade only VISIBLE pixels)
        └───────────────────────────────────┘
                         │
                         ▼
             [ On-Chip Frame Buffer ]
```

### 1.1 Tile Accelerator (Binning)
The screen is divided into a grid of **$32 \times 32$ pixel tiles**. During the geometry phase, the GPU's Tile Accelerator parses all polygons in the frame and bins them into local **Tile Lists** based on which tiles they intersect.

### 1.2 Image Synthesis Processor (ISP)
For each tile, the ISP processes the binned polygon list. It performs hidden-surface removal (depth calculation) on a per-pixel level *before* any texturing or shading occurs.

### 1.3 Texture and Shading Processor (TSP)
Once the ISP determines exactly which polygon surface is visible for each pixel, the TSP applies textures, lighting, and mipmapping *only* to those visible pixels. Overdraw is mathematically reduced to zero, maximizing pixel fillrate efficiency.

---

## 2. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_dreamcast_powervr.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_dreamcast_powervr.js).

This script:
1. Defines a scene with overlapping polygons at various depth (Z) levels.
2. Simulates the Tile Accelerator binning polygons into $32 \times 32$ tiles.
3. Implements the ISP's deferred depth sorting, identifying the single visible polygon for a target coordinate.
4. Computes comparative statistics illustrating the fillrate savings of TBDR vs. immediate-mode rendering.
