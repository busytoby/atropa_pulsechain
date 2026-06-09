# Unified Platform Display Architecture & Camera Models

To support different game genres (ranging from textual adventure interfaces to pseudo-3D racers and true 3D wireframe environments), this platform display model integrates multiple rendering modes and camera projection pipelines. 

By utilizing our host-side Vulkan presentation engine, we can swap between these display styles and camera modes dynamically without rewriting core execution logic.

---

## 1. Multi-Style Presentation Layer

The Vulkan pipeline is structured to interpret the memory registers in several distinct **Design Styles**:

```
                       ┌───────────────────────────────┐
                       │     Vulkan Presentation       │
                       │     Host Display Engine       │
                       └───────────────┬───────────────┘
                                       │
         ┌─────────────────────────────┼─────────────────────────────┐
         ▼                             ▼                             ▼
┌─────────────────┐           ┌─────────────────┐           ┌─────────────────┐
│  Terminal/Text  │           │  2D Tiled/Scroll│           │  Scanline 3D    │
│  - Char grids   │           │  - Sprites      │           │  - Pseudo-3D    │
│  - Status bars  │           │  - HSCROL/VSCROL│           │  - Road bend    │
└─────────────────┘           └─────────────────┘           └─────────────────┘
```

### 1.1 Terminal & Text Style (Interactive Fiction)
* **Description:** A text grid rendering characters from a font map.
* **Vulkan Pipeline:** Uses a character map texture as an index layer. The fragment shader resolves font glyphs directly from an ASCII character RAM buffer.
* **Layout:** Typically a fixed grid (e.g., 40x25 or 80x25 characters) with status overlays and terminal prompt feedback.

### 1.2 2D Tiled & Scrolling Style (Shamus / Rainbow Walker)
* **Description:** Orthographic tilemaps with hardware-assisted sprite composition.
* **Vulkan Pipeline:** Uses ANTIC-style Display List scrolling registers. The host binds specific texture atlases representing background grids and cycles colors dynamically.
* **Scroller registers:** Fine horizontal (`HSCROL`) and vertical (`VSCROL`) offsets are applied in the vertex shader to shift the viewport without rewriting the map buffer.

### 1.3 Pseudo-3D Scanline Style (Pole Position)
* **Description:** Linear horizontal shifting per scanline to project curved road planes and scaling lanes.
* **Vulkan Pipeline:** Modifies the horizontal translation parameter dynamically in the vertex shader for each scanline band. By loading a curvature lookup table, the shader shifts scanline coordinates to simulate curves:
  $$x_{\text{render}} = x_{\text{base}} + \Delta x_{\text{curve}}(y)$$

### 1.4 True 3D Wireframe Style (Mercenary)
* **Description:** Vector graphics mesh renderer utilizing vertex buffers and camera transformation matrices.
* **Vulkan Pipeline:** Performs real matrix transformations (rotation, translation, perspective projection) directly in the vertex shader or via fixed-point helper code. Renders wireframes using line list primitives (`VK_PRIMITIVE_TOPOLOGY_LINE_LIST`).

---

## 2. Dynamic Camera Model & Matrix Pipeline

To handle different perspectives, the system implements a unified matrix configuration register set starting at memory slot `55100`:

| Register Address | Name | Data Type | Description |
| :--- | :--- | :--- | :--- |
| `55100–55102` | **Cam Position `[X, Y, Z]`** | `int32` (Fixed-point 16.16) | Coordinates of the active camera |
| `55103–55105` | **Cam Rotation `[Pitch, Yaw, Roll]`** | `int16` (Euler Angles) | Orientation registers (degrees) |
| `55106` | **Camera Mode** | `uint8` | ID of active projection model |
| `55107` | **FOV / Scale** | `uint16` | Field of view angle or orthographic scale |

### 2.1 First-Person Perspective Cam (Mode `0`)
* **Use Case:** Cockpit view, flight simulators, or first-person exploration.
* **Projection:** Perspective division projects 3D coords onto the 2D plane:
  $$x' = \frac{X \cdot \text{FOV}}{Z} + \text{Center}_x$$
  $$y' = \frac{Y \cdot \text{FOV}}{Z} + \text{Center}_y$$

### 2.2 Third-Person Follow / Chase Cam (Mode `1`)
* **Use Case:** Racing games (*Pole Position*) and pseudo-3D jumpers (*Rainbow Walker*).
* **Projection:** The camera coordinates trace a target coordinate with a fixed distance ($d$) and elevation ($h$) offset:
  $$\vec{P}_{\text{cam}} = \vec{P}_{\text{player}} - \vec{D}_{\text{facing}} \cdot d + \vec{U}_{\text{up}} \cdot h$$
* The projection matrix applies dynamic tilt angles, making the camera lean in curves.

### 2.3 Orthographic Top-Down / Side Cam (Mode `2`)
* **Use Case:** 2D mazes (*Shamus*), maps, or tactical screens.
* **Projection:** Maps coords linearly, discarding depth calculations to render orthographic views:
  $$x' = X \cdot \text{scale} + \text{Offset}_x$$
  $$y' = Y \cdot \text{scale} + \text{Offset}_y$$

---

## 3. Implementation Blueprint in Vulkan Shaders

Our vertex shader pipeline dynamically handles these modes using the `CameraMode` register:

```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform CameraBlock {
    vec3 camPos;
    vec3 camRot;
    uint camMode;
    float scale;
} camera;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
    vec3 pos = inPosition - camera.camPos;
    
    // Apply Euler rotations (Yaw/Pitch)
    float cy = cos(camera.camRot.y);
    float sy = sin(camera.camRot.y);
    float cp = cos(camera.camRot.x);
    float sp = sin(camera.camRot.x);
    
    // Yaw
    float rx = pos.x * cy - pos.z * sy;
    float rz = pos.x * sy + pos.z * cy;
    // Pitch
    float ry = pos.y * cp - rz * sp;
    rz = pos.y * sp + rz * cp;

    if (camera.camMode == 0 || camera.camMode == 1) {
        // Perspective Camera Modes (First & Third Person)
        float zDepth = rz + 1.0; // Avoid division by zero
        gl_Position = vec4((rx * camera.scale) / zDepth, (ry * camera.scale) / zDepth, zDepth / 1000.0, 1.0);
    } else {
        // Orthographic Camera Mode (2D / Map)
        gl_Position = vec4(rx * camera.scale, ry * camera.scale, 0.0, 1.0);
    }
}
```

---

> [!NOTE]
> By offloading projection decisions to the host shader using the `CameraMode` register, the guest game loop merely updates object coordinates in memory, completely bypassing CPU math bottlenecks.
