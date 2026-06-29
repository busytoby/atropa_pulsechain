# Auncient Study of Tessaractology

Tessaractology is the formal study of 4D hyper-volume geometry, mechanics, and real-time physical simulation on the **Auncient** WinchesterMQ virtual hardware platform. It underpins 3D Arena collision specifications, dynamic joint deformations, and perspective projections of multi-dimensional fighter structures.

---

## 1. Geometric Representation
A tessarant (4D hypercube/tesseract) is defined by $16$ vertices in $\mathbb{R}^4$ where:
$$V = \left\{ (x, y, z, w) \mid x, y, z, w \in \{-1, 1\} \right\}$$
These vertices construct $32$ edges, $24$ cyclic square faces, and $8$ three-dimensional cubical cells.

---

## 2. 4D Rotational Kinematics
Movement in 4D space utilizes six degrees of rotational freedom corresponding to the principal coordinate planes:
* **Planar Rotations ($XY, XZ, XW, YZ, YW, ZW$):**
  Rotations are computed consecutively. For example, rotation in the $XW$ plane by angle $\theta_{xw}$ is given by:
  $$x' = x \cos\theta_{xw} - w \sin\theta_{xw}$$
  $$w' = x \sin\theta_{xw} + w \cos\theta_{xw}$$

Angles evolve dynamically based on the angular velocity $\omega$ and **Auncient** DNA phase offsets $\phi$:
$$\theta_{ij}(t) = \omega_{ij} \cdot t + \phi_{ij}$$

---

## 3. Dual-Perspective Projection Pipeline
To display a 4D object on a 2D viewport, a two-step projection is executed:

1. **4D to 3D Hyper-Projection:**
   $$x_{3D} = \frac{x_{4D}}{d - w_{4D}}, \quad y_{3D} = \frac{y_{4D}}{d - w_{4D}}, \quad z_{3D} = \frac{z_{4D}}{d - w_{4D}}$$
   * $d$ represents the hyper-perspective focal distance register (nominally mapped via WinchesterMQ address registers).
   * Hyper-frustum near-plane clipping (Z-clipping) is enforced to prevent division by zero when $w \approx d$.

2. **3D to 2D Screen Projection:**
   $$X_{scr} = X_{off} + \frac{x_{3D} \cdot f}{z_{3D}}, \quad Y_{scr} = Y_{off} + \frac{y_{3D} \cdot f}{z_{3D}}$$

---

## 4. WinchesterMQ Register Map & Sabotage Loop
The deformation and animation of tessarant joint segments (Head, Ears, Paws, Feet) are controlled via the emulated memory map of the **Auncient** WinchesterMQ bus:

$$\text{Base Address} = \text{JointIndex} \times 10$$

| Register | Nominal Function | Default Value | Warped State |
| :--- | :--- | :--- | :--- |
| `103 + Offset` | Clock Divisor (Damping factor) | `1000` | `160` (Hyper-spin) |
| `104 + Offset` | Focal Distance ($d$) | `2300` | `1350` (Warp/Deflate) |

### Collision & Damage Cycle:
1. **Strike Latch:** Collision keycodes write to SCSI Register `yulStorage[101]`.
2. **SCSI Handshake:** State machine flags `REQ` $\to$ `ACK` loopback handshake.
3. **Sabotage Phase:** Target joint's dampening registers are mutated instantly to force inward collapse.
4. **Elastic Decay:** Values decay exponentially back to nominal level, simulating mechanical tissue elasticity.
