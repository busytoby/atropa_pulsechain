# Game Archetypes for Automated Vulkan Rendering Pipelines

This specification defines the coordinate registers, state databases, and graphics pipelines for **Candy Bandit** (2D Platformer/Maze style) and **Speed Racer** (Pseudo-3D Racer style). 

These archetypes serve as a baseline model for developing automated presentation-layer rendering systems in our Vulkan framework.

---

## 1. Candy Bandit Archetype (2D Platformer & Grid)

*Candy Bandit* represents a classic multi-screen platform/maze action game. The state represents tile structures, player coordinates, and enemy motion vectors.

```
       Logical Viewport (320 x 200) -> Mapped to 40 x 25 Char Grid
  ┌────────────────────────────────────────────────────────┐
  │ [Score: 00240]                       [Candies: 04 / 08]│
  │                                                        │
  │     #============================================#     │
  │     #                                            #     │
  │     #    + (Player)                              #     │
  │     #                                            #     │
  │     #                o (Candy)     X (Guard)     #     │
  │     #============================================#     │
  └────────────────────────────────────────────────────────┘
```

### 1.1 Memory Register Layout (Storage Namespace `3000600+`)

| Storage Slot | Name | Data Type | Description |
| :--- | :--- | :--- | :--- |
| `3000600` | **Player X** | `uint16` | Horizontal position (0–319 pixel space) |
| `3000601` | **Player Y** | `uint16` | Vertical position (0–199 pixel space) |
| `3000602` | **Player Action State** | `uint8` | `0` = Stand, `1` = Walk, `2` = Jump, `3` = Climb |
| `3000603` | **Active Score** | `uint32` | Total candy score accumulated |
| `3000604` | **Level Candy Count** | `uint8` | Remaining candies on the screen |
| `3000610–3000617` | **Candy Coords `[X, Y]`** | `uint8` packed | Coords of up to 8 active candy items |
| `3000620–3000635` | **Guard Databases** | packed `uint256` | 4 guards: Coords, facing direction, patrol limits |

### 1.2 Automated Vulkan Rendering Mapping
* **Background Tilemap Shader**: Parses a 1000-byte character RAM block. The vertex shader divides the viewport into a 40x25 grid, and the fragment shader overlays brick and ladder textures based on the byte values.
* **Sprite Pipeline**: The automated system monitors slots `3000600` (Player) and `3000620` (Guards). It instantiates Vulkan billboard primitives at the calculated screen offsets, matching the current frame state (e.g. walk/climb frames) without CPU intervention.

---

## 2. Speed Racer Archetype (Pseudo-3D Scanline racer)

*Speed Racer* models a high-speed racing environment utilizing a pseudo-3D scanning road layout.

```
                  Horizon (Scanline 80)
  ────────────────────────────────────────────────────────
                          /\  (Mountain background)
                         /  \
                        /    \
                       /  ||  \
                      /   ||   \
                     /    ||    \  (Opponent vehicle)
                    /     ||     \
                   /     [██]     \
                  /       ||       \
                 /        ||        \
  ──────────────/        /  \        \────────────────────
               /        /    \        \
              /        /      \        \
```

### 2.1 Memory Register Layout (Storage Namespace `3000700+`)

| Storage Slot | Name | Data Type | Description |
| :--- | :--- | :--- | :--- |
| `3000700` | **Vehicle Speed** | `uint16` | Forward speed scaling factor |
| `3000701` | **Vehicle Steer Offset**| `int16` | Steer offset relative to road center |
| `3000702` | **Distance Traveled** | `uint32` | Metric distance along the active track |
| `3000703` | **Current Curvature** | `int16` | Target curve rate (negative is left, positive right) |
| `3000710–3000719` | **Opponent Positions** | packed `uint256` | Track offsets, lane positions, speeds for 3 cars |
| `3000720–3000959` | **Road Curve Lookup** | `int8` Array | 240 bytes holding curvature offsets per screen line |

### 2.2 Automated Vulkan Rendering Mapping
* **Road Projection Shader**: Renders the road lines dynamically. Instead of pushing a static mesh, the shader reads the **Road Curve Lookup** buffer (`3000720+`) and the `VehicleSteerOffset`.
* **Scanline Offset Shader**:
  For each scanline index $y$ representing the road region (scanline 80 to 239):
  $$X_{\text{center}}(y) = \text{Center}_x + \text{CurveLookup}[y - 80] - \text{SteerOffset} \cdot \text{scale}(y)$$
  The fragment shader renders alternating green/brown side grass bands and grey asphalt lanes relative to $X_{\text{center}}(y)$.
* **Sprite Scaling**: Opponent cars are rendered as sprites. The system reads the opponent distance relative to the player, dynamically scaling the sprite scale in the vertex shader:
  $$\text{Scale} = \frac{\text{SpriteScale}_{\text{base}}}{\text{Distance} + 1.0}$$

---

> [!NOTE]
> By mapping these structures to explicit storage slots, the host presentation system can implement optimized pipeline shaders specifically tuned for 2D platforming (scrolling billboard sprites) or pseudo-3D racing (scanline-curved road rendering).
