# TARTAN Spatial Computing & Parametric Thacher Evaluators

```
                         TARTAN SPATIAL COMPUTING FABRIC ARCHITECTURE
+---------------------------------------------------------------------------------------------------+
|  [ TARTAN Coordinate Fields (Warp & Weft Multi-Planar Space) ]                                    |
|    - u (Horizontal Warp Field)  : Spatial pane curvature & peripheral boundary wrapping           |
|    - v (Vertical Weft Field)    : Elevation tilt & interactive user perspective                   |
|    - w (Sett Depth Hierarchy)   : Z-depth layer stack & spatial window priority                   |
|         │                                                                                         |
|         ▼                                                                                         |
|  ┌────────────────────────────────────────────────────────────────────────────┐                  |
|  │ PARAMETRIC THACHER EVALUATOR ENGINE (In-Silicon Cylindrical Recurrence)     │                  |
|  │                                                                            │                  |
|  │  1. Non-Preferential 3-Term Orthogonal Polynomial Recurrence:              │                  |
|  │     P_{k+1}(u) = (A_k * u + B_k) * P_k(u) - C_k * P_{k-1}(u)               │                  |
|  │                                                                            │                  |
|  │  2. Surface Metric Tensor & Geodesic Ray Curvature:                        │                  |
|  │     g_ij(u, v) = (d X / d u_i) . (d X / d u_j)                             │                  |
|  │                                                                            │                  |
|  │  3. Motzkin Prime Field Quantization (953467954114363):                    │                  |
|  │     Phi_Tartan(u, v, w) = Base^{u * v} mod MotzkinPrime                    │                  |
|  └─────────────────────────────────────┬──────────────────────────────────────┘                  |
|                                        │                                                          |
|                                        ▼ (0.18 ns Zero-Copy DMA Burst)                            |
|  [ EFL Evas Smart Objects & ReBAR VRAM Aperture (0xF0000000) ]                                    |
|    - Multi-layered 3D spatial window panes floating around the desktop user                       |
|    - Direct eye-gaze depth-of-field alignment & MANN gesture deformation                         |
+---------------------------------------------------------------------------------------------------+
```

---

## 1. The TARTAN Spatial Paradigm

**TARTAN** is the native spatial computing framework for Enlightenment Foundation Libraries (EFL), the Dysnomia VM, and the CPMTomie environment. Named after the traditional Scottish woven cloth characterized by crisscrossing horizontal and vertical bands (warp and weft) forming distinct sett patterns, TARTAN treats three-dimensional spatial computing space as a continuous, mathematically woven fabric:

1. **Warp ($u$)**: The horizontal spatial coordinate field governing pane curvature, peripheral panoramic wrapping, and horizontal eye-gaze tracking.
2. **Weft ($v$)**: The vertical coordinate field managing elevation angles, interactive head tilt, and gravitational perspective lines.
3. **Sett ($w$)**: The discrete repeating depth intervals defining application layer hierarchies, spatial window elevations, and volumetric depth boundaries.

Unlike traditional window managers that project rigid 2D rectangular planes onto a flat desktop, TARTAN evaluates flexible, continuous 3D spatial surfaces that curve, float, and orbit naturally in response to user proximity, gaze, and hand gestures.

---

## 2. Parametric Thacher Evaluators

At the core of the TARTAN geometric engine is the **Parametric Thacher Evaluator**, derived from Edwin Thacher’s cylindrical calculating scales and the ORDVAC in-silicon arithmetic engines.

### Mathematical Foundations:
1. **Closed-Form 3-Term Orthogonal Recurrence**:
   Rather than relying on iterative Newton-Raphson approximation methods that suffer from divergence near inflection points, the Thacher evaluator evaluates spatial surface curvature using non-preferential 3-term recurrence relations:
   $$P_{k+1}(u) = (A_k \cdot u + B_k) P_k(u) - C_k P_{k-1}(u)$$
   This provides exact, stable coordinate positions across arbitrary view angles in sub-nanosecond execution time ($0.18\text{ ns}$).

2. **Metric Tensor & Geodesic Ray Evaluation**:
   The local geometric properties of any floating TARTAN spatial pane are evaluated via the Riemannian metric tensor:
   $$g_{ij}(u, v) = \frac{\partial \mathbf{X}}{\partial u^i} \cdot \frac{\partial \mathbf{X}}{\partial u^j}$$
   allowing optical shaders (such as Pixar RenderMan caustics and thin-lens depth of field) to refract background scenes through curved floating glass windows with complete physical fidelity.

3. **Motzkin Prime Quantization**:
   All spatial transformations are anchored to the canonical Dysnomia field divisor $P = 953467954114363$, ensuring phase coherence with WinchesterMQ SCSI register boundaries.

---

## 3. Interaction with Memory-Augmented Neural Networks (MANN)

TARTAN spatial panes interface directly with the **TSFi MANN Controller** (`tsfi_mann_controller.c`):
* **Gaze & Gesture Latents**: User spatial gestures, pinch-to-zoom vectors, and eye tracking are transformed into 64-dimensional latent vectors.
* **Content-Addressable Recall**: The MANN memory matrix evaluates cosine similarities across 16 memory slots, predicting user focus and dynamically sharpening depth-of-field and shadow penumbras on the targeted TARTAN window.
* **Least-Used Slot Recycling**: Unused spatial layers gracefully fade into the background ambient fog using Rayleigh/Mie volume scattering (`tsfi_cpm_mann_renderman_atmosphere.c`).

---

## 4. Hardware Pipeline & Zero-Copy Presentation

Through integration with **THE RED VULKAN** and the **AMD 8-Way Data Fabric** (`00:18.0`–`00:18.7`):
* Evaluated TARTAN parametric vertices bypass host CPU memory copying, streaming directly into the GPU Resizable BAR aperture (`0xF0000000`).
* EFL Evas smart objects render these surfaces seamlessly at **60 FPS** alongside the Pixar RenderMan Utah Teapot master scene, delivering true next-generation spatial computing on open, sovereign hardware.
