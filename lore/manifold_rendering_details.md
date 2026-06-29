# Rendering Telemetry & Micro-Animation Details

To render a cycle-accurate, high-fidelity visualizer of the 9-epoch manifold reaction pipeline, the rendering engine must track hundreds of discrete simulation details. This document acts as the definitive specification for the coordinate grids, particle parameters, and HUD telemetry systems.

---

## 1. Grid Mesh: 2048 Leaves / 11 Reduction Levels

The Banach-Hilbert substrate is represented as a structured grid warp. The rendering engine tracks the coordinates of **2048 leaf intersections** ($32 \times 64$ grid nodes).

*   **Node Coordinates**: Each node is stored as $P_{i,j} = (x_0 + \Delta x_{i,j}, y_0 + \Delta y_{i,j})$ where $i \in [0, 32], j \in [0, 64]$.
*   **Grid Distortion Matrix**: Let $\vec{D}_{i,j}$ be the displacement vector applied to node $(i,j)$.
    *   *Default*: $\vec{D}_{i,j} = 0$.
    *   *Epoch 3 (Polarize)*: Warp nodes based on proximity to the active orthogonal axis:
        $$\vec{D}_{i,j} = \left(\sin(2\pi \cdot f \cdot y) \cdot A, 0\right) \text{ near Rod}$$
    *   *Epoch 5 (Conify)*: Node displacements point towards the center $(x_c, y_c)$ with magnitude proportional to the square root of the distance:
        $$\vec{D}_{i,j} = \left(\frac{x_c - x}{|x_c - x|}, \frac{y_c - y}{|y_c - y|}\right) \cdot \text{Foundation}$$

---

## 2. Verlet Particle Parameters (150-300 Particle Cloud)

Each particle in the emitter cloud requires tracking **9 individual variables** to maintain physical accuracy:

| Parameter | Type | Range | Description |
| :--- | :--- | :--- | :--- |
| **`x`** | Float | `0.0` to `CanvasWidth` | Current horizontal coordinate. |
| **`y`** | Float | `0.0` to `CanvasHeight` | Current vertical coordinate. |
| **`px`** | Float | `0.0` to `CanvasWidth` | Previous horizontal coordinate (for velocity evaluation). |
| **`py`** | Float | `0.0` to `CanvasHeight` | Previous vertical coordinate. |
| **`mass`** | Float | `0.5` to `3.0` | Inertial resistance to field acceleration. |
| **`drag`** | Float | `0.92` to `0.98` | Velocity damping factor (simulating dielectric viscosity). |
| **`size`** | Float | `1.0` to `4.0` pixels | Renders visual prominence of high-mass components. |
| **`color`** | String / Hex | `#39ff14`, `#00f0ff`, `#ffd700` | Current wavelength status of the particle. |
| **`phase`** | Float | `0.0` to $2\pi$ | Individual oscillation offset for sinusoidal path variations. |

---

## 3. Wavefield Amplitude Mapping

To display wave interference patterns (Epoch 1 & 7), the background contains a continuous field intensity map evaluated at coordinate $(x,y)$:
$$I(x,y,t) = \sin(k \cdot d_{\text{rod}} - \omega t) \cdot A_{\text{rod}} + \sin(k \cdot d_{\text{cone}} - \omega t) \cdot A_{\text{cone}}$$
*   **$d_{\text{rod}}$ / $d_{\text{cone}}$**: Euclidean distance from $(x,y)$ to the Rod and Cone node centers.
*   **$k$**: Spatial frequency (wave number, derived from `Base`).
*   **$\omega$**: Temporal frequency (derived from `Secret`).
*   **Intensity Rendering**: The background uses a screen-blend shader mapping $I(x,y,t)$ to a gradient scale from transparent black to glowing cyan/green.

---

## 4. Real-Time Telemetry Log (HUD Dashboard)

The surrounding panel outputs real-time metrics tracking the internal computational load of the Dysnomian Labyrinth:

```
[SYSTEM REGISTER PROBE: DYSNOMIA_VM]
------------------------------------
Core Flux Intensity  : [=======-----] 57.65% -> 87.65%
Wavefield Stability  : 44.20% (UNSTABLE: EPOCH 0-7) | 100.00% (LOCKED: EPOCH 8)
Transition Delay     : 6.94 ms
Active Registers     : 144 Fraunhofer Grans (Hilbert Fibers)
W^X Security Lock    : RW [ACTIVE] | RX [ENFORCED]
```

### Telemetry Value Transitions

| Epoch | Flux % | Stability % | Register Readout | Frame Rate Target |
| :--- | :--- | :--- | :--- | :--- |
| **0: INIT** | `24.0%` | `10.0%` | Random Hex strings updating at 60Hz | 60 FPS |
| **1: AVAIL** | `35.5%` | `15.0%` | `Contour` hashes static, `Xi` pulsing | 60 FPS |
| **2: FORM** | `48.0%` | `22.5%` | Collision peaks | 45 FPS (simulated lag) |
| **3: POLARIZE** | `55.0%` | `30.0%` | Axis coordinate grids locked | 60 FPS |
| **4: CONJUGATE**| `62.0%` | `40.0%` | `Coord` crossing packet values | 60 FPS |
| **5: CONIFY** | `70.0%` | `55.0%` | Spiral reduction index count down | 60 FPS |
| **6: SATURATE** | `82.0%` | `70.0%` | Swelling variables peaking | 50 FPS |
| **7: IONIZE** | `90.0%` | `85.0%` | High-frequency plasma load | 60 FPS |
| **8: MAGNETIZE**| `100.0%` | `100.0%` | Golden Static Mirror Key (`Ring`) | Static / Frozen |

---
> [!TIP]
> To optimize rendering performance when simulating the 2048 leaf coordinate intersections alongside the 300 Verlet particles, use instanced grid rendering on a single canvas path buffer rather than individual path drawing instructions.
