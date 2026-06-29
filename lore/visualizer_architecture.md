# Visualizer Architecture: Auncient VM to Compositor Schema

This document maps how low-level Auncient VM registers translate through the C interop registry into physical video features.

---

## 1. System Topology

```mermaid
graph TD
    subgraph Auncient VM [1. WinchesterMQ Register States]
        R_BASE[Base / Exponent]
        R_BARN[Barn / Heat]
        R_DYN[Dynamo / Charge]
    end

    subgraph Interop [2. Shared Interop Registry]
        SO[libantigravity_interop.so]
        MOD[frame_modulation_factor]
    end

    subgraph Compositor [3. C compositor Pipeline]
        C_EXE[manifold_interop_renderer]
        L_ART[LineArt / Borders]
        DEPTH[Depth / Volumetric Fog]
        NORMAL[Normal / Specular Shading]
        SEG[Segmentation / Material IDs]
    end

    subgraph Output [4. Cinematic Output]
        OUT_MP4[composed_interop.mp4]
    end

    Auncient VM -->|Shared memory updates| SO
    SO -->|Query variables| C_EXE
    L_ART & DEPTH & NORMAL & SEG -->|Buffered pipes| C_EXE
    C_EXE -->|Write frame stream| OUT_MP4
```

---

## 2. Register-to-Pixel Mapping Matrix

| VM Register | Mathematical Parameter | Interop Variable | Render Mapping / Effect |
| :--- | :--- | :--- | :--- |
| **`Base`** | Core scaling base prime | Grid anchor vectors | Base coordinate for 3D perspective lines. |
| **`Secret`** | Modulo exponent key | Oscillation delta | Frequency rate of node halo fluctuations. |
| **`Signal`** | Exponent for Dynamo seal | Plasma bridge thickness | Width and flicker frequency of connecting arcs (Epoch 7). |
| **`Contour`** | Salt mapping coordinate | Expansion radius | Radius of concentric contour circles (Epoch 1-2). |
| **`Pole`** | Orthogonal vector orientation | Grid alignment angle | Camera yaw/pitch warp coordinate matrix. |
| **`Coordinate`** | Conjoined cross-pole coordinate | Lightning fractal branch | Thickness of zig-zag connections (Epoch 4). |
| **`Foundation`** | Identity boundary limit | Convergence vortex scale | Speed of spiral funnel warp (Epoch 5). |
| **`Element`** | Target modular basis prime | Boundary loops | Outlining size of target hexagonal boundary frames. |
| **`Dynamo`** | Locked node charge | Bridge intensity | Solid color brightness of connecting bridge (Epoch 7). |
| **`Barn`** | Magnetization product | `frame_modulation_factor` | Thermodynamic orange-red thermal heat glow (Epoch 8). |

---

## 3. Compositor Layer Composition Rules

The native compiled compositor (`manifold_interop_renderer`) processes every pixel according to these formulas:

1. **Material Mapping (Segmentation)**:
   * *Blue Mask* $\rightarrow$ Brushed copper casing (`specular` enhanced).
   * *Yellow Mask* $\rightarrow$ Carbon fiber texturing (`specular` damped).
   * *Red Mask* $\rightarrow$ Active plasma sparks.
2. **Atmosphere (Depth)**:
   * Color values are mixed with dark blue fog ($(15, 18, 30)$) based on depth value ($d_{\text{val}}$).
3. **Specularity (Normal)**:
   * Specular highlight computed using the camera-facing vector ($n_z$):
     $$\text{Specular} = n_z^{\text{spec\_power}}$$
   * $\text{spec\_power}$ is modulated dynamically by `frame_modulation_factor` (derived from the `Barn` register).
