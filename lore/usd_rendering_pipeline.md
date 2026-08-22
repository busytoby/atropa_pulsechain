# Pixar USD 3D Asset rendering pipeline

This document details the generation, animation, and rendering pipeline of Pixar USD assets for Stuffed Teddy Bears within the **Auncient** simulation suite.

---

## 1. Asset Structure & Hierarchy
The system represents the teddy bear character as a unified Pixar USD `SkelRoot` that inherits properties from the global schemas:

```usd
def SkelRoot "TeddyBearSkelCharacter" (
    inherits = </AuncientTeddyBearSchema>
)
```

The skeletal structure is divided into discrete, referenceable mesh components stored in `tsfi2-deepseek/assets/`:
* `Head` (FurMaterial)
* `LeftEar` / `RightEar` (FurMaterial)
* `LeftEye` / `RightEye` (ButtonMaterial)
* `Muzzle` / `Nose` (ButtonMaterial)
* `SewnHeart` (ClothMaterial)
* `LeftUpperArm` / `RightUpperArm` (FurMaterial)
* `LeftForearm` / `RightForearm` (FurMaterial)
* `LeftThigh` / `RightThigh` (FurMaterial)
* `LeftCalf` / `RightCalf` (FurMaterial)

---

## 2. Material Bindings & Shader Interop
USD materials are organized inside a global `Looks` scope. Each component binds to its respective shader surface:

* **FurMaterial**: Default PBR preview surface representing brown bear fur (`diffuseColor = (0.47, 0.31, 0.21)`, `roughness = 0.95`, `metallic = 0.0`).
* **ClothMaterial**: High-contrast red fabric for the heart mesh (`diffuseColor = (0.78, 0.12, 0.16)`, `roughness = 0.85`).
* **ButtonMaterial**: High-reflectivity black plastic/glass representing eyes and muzzle details (`diffuseColor = (0.08, 0.08, 0.08)`, `roughness = 0.1`, `metallic = 0.9`).

Dynamic shader properties (like albedo changes and specular highlights) are calculated via [TSFiUsdShade](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_usdshade.h) dynamically driven by Verlet physics discharge loops.

---

## 3. Kinematics & Animation
Joint transforms are written out as time-sampled coordinates mapping to the animation frames:
* **Translation Samples**: `double3 xformOp:translate.timeSamples`
* **Rotation Samples**: `double3 xformOp:rotateXYZ.timeSamples`

The walking geometry ([generate_usd_teddy_bear_walk.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_usd_teddy_bear_walk.py)) calculates these samples using periodic sine/cosine transformations to simulate joint swings, torso rolls, and knee bends over a 24-second cycle.

---

## 4. Video Rendering Flow
1. **Geometry Evaluation**: Evaluates 3D ellipsoid vertices for each body segment at timestep $t$.
2. **Camera Projection**: Projects 3D global coordinates $(x, y, z)$ into 2D grid screen space $(sx, sy)$ using a perspective field of view (FOV) algorithm:
   $$sx = \text{width}/2 + \frac{x \cdot \text{FOV}}{z}$$
   $$sy = \text{height}/2 - \frac{y \cdot \text{FOV}}{z}$$
3. **Specularity & Fur Rendering**: Draws specular highlights on boundary edges (e.g. blue highlights on the Head) and overlays procedural curves to render fur strands.
4. **FFmpeg Compilation**: Pipes generated frame buffers to `ffmpeg` to compile the final `.mp4` video container with the walking audio track.
