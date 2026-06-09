# Mercenary 3D Map, Room Objects, & World Hollow Architecture

This document details the on-chain representation, 3D geometry engine, and interactive database structures modeling Novagen’s ***Mercenary: Escape from Targ*** (1985) within the TSFi2 virtual ecosystem.

---

## 1. 3D Map Geometry & World Hollow Coordinate Space

The planetary coordinate system for Targ and the subterranean complexes (such as the World Hollow/Central Complex) maps to a local 3D Cartesian space.

### 1.1 Emulated Vertex Map (17 Vertices)
The 3D wireframe mesh representing the environment (ground grid, primary target building, and flying spaceship) is stored in Yul CPU VM memory slots starting at `55280` (`$D7F0`):

| Vertex Index | Coordinate `[X, Y, Z]` | Component Description | Address Range (Lo, Mid, Hi) |
| :---: | :---: | :--- | :--- |
| **0–7** | `[-20, -30, -20]` to `[20, 20, 20]` | Main Cuboid Target Building | `55280 - 55303` |
| **8–11** | `[-80, -45, -80]` to `[-80, -45, 80]` | Ground Coordinate Grid bounds | `55304 - 55315` |
| **12–16** | `[0, 60, 0]` to `[-15, 35, 15]` | Spaceship (Apex & Base Pyramid) | `55316 - 55330` |

### 1.2 Line Connection Registry (24 Lines)
Line connection mappings are stored starting at address `55536` (`$D8F0`):
*   **Building Outlines (Lines 0–11)**: Front face, back face, and vertical columns.
*   **Ground Outlines (Lines 12–15)**: Four lines defining boundary limits.
*   **Spaceship Outlines (Lines 16–23)**: Base connections and apex struts.

---

## 2. On-Chain Room & Object Database Schema

Subterranean exploration in the World Hollow requires a structured database mapping item positions, active locks, and room layouts.

```
                      +---------------------------------------+
                      |         Novagen Room Database         |
                      |   Stored at slot: 2000000 + objId     |
                      +---------------------------------------+
                        /                 |                 \
                       /                  |                  \
  +-------------------------+  +----------------------+  +-------------------------+
  |    Object Coordinates   |  |   Ownership Status   |  |     Active Flags        |
  |     - X / Y Position    |  |   - Room ID (0-255)  |  |     - Can Pick Up       |
  |     - Z (Depth) Layer   |  |   - Player Held Bit  |  |     - Triggers Sound    |
  +-------------------------+  +----------------------+  +-------------------------+
```

### 2.1 Storage Allocation Matrix
To prevent collisions inside the Ethereum Virtual Machine, we partition the game state into namespaces:

| Slot Range | Namespace | Purpose | Data Struct |
| :--- | :--- | :--- | :--- |
| `2000000–2000049` | **Room Inventory** | Maps item IDs located inside specific rooms | `uint8` Array |
| `2000050–2000099` | **Object Metadata** | Storage for items (e.g. keycards, energy packs) | Packed `uint256` |
| `2000100–2000255` | **Door Lock States** | Maps lock colors (Blue, Red, Yellow) to rooms | Bitmask |

### 2.2 Object Metadata Word Layout
Each item in the Targ database is packed into a single 256-bit word at `2000000 + objId`:
*   `[0..7]`: Current Room ID (0 represents player inventory).
*   `[8..23]`: Packed local X coordinate.
*   `[24..39]`: Packed local Y coordinate.
*   `[40..55]`: Packed local Z coordinate.
*   `[56..63]`: Interaction mask (e.g., `0x01` = collectible, `0x02` = keycard, `0x04` = energy source).

---

## 3. The Yul 3D Camera Projection Pipeline

The wireframe rendering pipeline uses trigonometric lookup tables and fixed-point math to project the 3D vertices onto the 2D C64 high-resolution viewport (320x200):

1.  **Yaw & Pitch Rotation**:
    $$\begin{aligned}
    X' &= X \cos(\text{yaw}) - Z \sin(\text{yaw}) \\
    Z' &= X \sin(\text{yaw}) + Z \cos(\text{yaw})
    \end{aligned}$$
2.  **Perspective Division**:
    $$x_{\text{proj}} = \frac{X' \cdot \text{scale}}{Z' + \text{depth}} + \text{offset}_x$$
    $$y_{\text{proj}} = \frac{Y' \cdot \text{scale}}{Z' + \text{depth}} + \text{offset}_y$$
3.  **Rasterization Strobe**:
    Writing `1` to memory address `54966` triggers the GPU/ReBAR projection thunk. If successful, the engine stores the final screen line coordinates starting at `55800` and sets register `54967` to `1`.
