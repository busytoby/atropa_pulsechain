# Sierra 3D Wireframe Projection Engine (Vulkan & DeepSeek Coordinate Alignment)

This document details the virtual hardware, registers, and Yul contract interface design for the **Sierra 3D Wireframe Projection Engine** emulation on the TSFi2 platform, coordinating with Vulkan frame buffers and DeepSeek alignment consensus checks.

---

## 1. 3D Wireframe Coprocessor Memory Map ($D6B0–$D6BF)

The 3D Wireframe projection coprocessor registers reside at `$D6B0`–`$D6B7` (54960–54967):

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6B0` | `54960` | Read / Write | **Rotation X (Pitch)**: X-axis angle (0–15 representing 16 steps of $22.5^\circ$). |
| `$D6B1` | `54961` | Read / Write | **Rotation Y (Yaw)**: Y-axis angle (0–15). |
| `$D6B2` | `54962` | Read / Write | **Rotation Z (Roll)**: Z-axis angle (0–15). |
| `$D6B3` | `54963` | Read / Write | **Scale Modifier**: Projection zoom scale (default 100). |
| `$D6B4` | `54964` | Read / Write | **Vertex Count**: Number of 3D vertices to transform (1–32). |
| `$D6B5` | `54965` | Read / Write | **Line Count**: Number of segments to connect (1–32). |
| `$D6B6` | `54966` | Write-Only | **Compute Trigger**: Writing `1` initiates transformation and renders directly to the Doodle Graphics framebuffer. |
| `$D6B7` | `54967` | Read-Only | **Status Register**: Execution output (0 = Idle, 1 = Success). |

### Buffers:
* **Vertex List (`$D7F0`–`$D8EF` / `55280`–`55535`)**: 32 vertices, each using 3 bytes: $X$ (signed 8-bit), $Y$ (signed 8-bit), $Z$ (signed 8-bit).
* **Line Connections List (`$D8F0`–`$D9CF` / `55536`–`55759`)**: 32 line entries, each using 2 bytes: `from_vertex_index` (0-31), `to_vertex_index` (0-31).

---

## 2. Dynamic 3D Matrix Rotations & Projections (Yul)

When trigger strobe `$D6B6` is poked with `1`, the Yul engine:

1. **Calculates Fixed-point Trigonometric Angles**: Uses on-chain scaled $\sin$ / $\cos$ lookup helper functions.
2. **Rotates 3D Coordinates**: Performs matrix calculations for pitch, yaw, and roll.
3. **Projects to 2D Perspective Screen Coordinates**:
   $$x' = 160 + \frac{X \cdot \text{scale}}{Z + \text{distance}}$$
   $$y' = 100 + \frac{Y \cdot \text{scale}}{Z + \text{distance}}$$
4. **Renders to Doodle Graphics Framebuffer**: Invokes internal `drawLine` routines to draw lines onto the visual display.
5. **Vulkan Pipeline & DeepSeek Consensus Integration**:
   - The Doodle Graphics buffer updates are synced directly to Vulkan's textures on the native visualizer window.
   - The engine checks for a consensus key inside `acousticOracle` (latestRoot) and validates alignment before processing.
   - Emits the event log:
     ```solidity
     event WireframeRendered(address indexed user, uint256 lineCount);
     ```
     Topic: `0xee6c1cb417ad0a8b94df8c8a14b0368b634882dfba51912bc5527e0291f24d1a`.
