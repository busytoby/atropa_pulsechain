# 3D Stuffed Animal Rendering & Telemetry Evolution

The RAG node visualizer has been evolved from simple flat 2D shapes into interactive, rotating, 3D ray-marched stuffed animals rendered in real-time under a software-rendered Wayland compositor.

## 📐 Signed Distance Field (SDF) Architecture

To keep rendering lightweight and fast enough for software execution on the CPU, each stuffed animal is modeled using a collection of mathematically blended primitive shapes (ellipsoids, spheres, and cones) using a smooth minimum function ($smin$).

### 1. The 3D Teddy Bear (Baseline K0Rn)
Formulated by combining ellipsoids for the body, head, snout, and ears:
$$d_{\text{body}} = \sqrt{1.2x^2 + 0.8y^2 + 1.2z^2} - 0.35$$
$$d_{\text{head}} = \sqrt{x^2 + (y-0.35)^2 + z^2} - 0.25$$

### 2. The 3D Crow
Uses custom wing ellipsoids and a sharp cone for the beak:
$$d_{\text{beak}} = \sqrt{5(x-0.45)^2 + 5(y-0.2)^2 + 5z^2} - 0.1$$

### 3. The 3D Cat
Employs pointy ear shapes and a tail sphere:
$$d_{\text{ear}} = \sqrt{2(x-0.35)^2 + 2(y-0.38)^2 + 2z^2} - 0.05$$

---

## 🎨 Evolved Interface Preview

Below is the design mockup displaying the force-directed RAG nodes rendered using the active 3D Raymarching engine:

![3D Stuffed Animals Layout](/home/mariarahel/.gemini/antigravity-cli/brain/5289e240-c025-43c9-95f2-79673251a341/3d_stuffed_animals_layout_1780879613847.png)

---

## 🛠️ Code Implementation
The code is fully integrated into the graphics pipeline:
* **Types Extended**: Introduced `GFX_STUFFED_3D` within `GfxType` in [tests/test_wayland_terminal_shell.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_terminal_shell.c).
* **Raymarch Loop**: Implemented `draw_3d_stuffed_animal` to cast rays for each node pixel, evaluate lighting normals using finite differences, and shade using Dracula-themed colors.
* **Continuous Rotation**: Driven by incrementing `g_frame_counter` on every Wayland swap-buffer frame callback.
