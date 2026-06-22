# Hugi 4D Graphics Integration — Technical Blueprint

This document details how classical demoscene algorithms from **Hugi Diskmag** (such as the Coding Section and *Hugi SE #01*) are integrated into our **Auncient** 4D tesseract rendering engine inside `teddy_bear_tournament_3d.html`.

---

## 1. 6-Axis 4D Rotations

In 3D graphics, rotations occur across three coordinate planes (XY, XZ, YZ). In 4-dimensional space, rotations generalize to six orthogonal planes:
*   **3D Planes**: XY, XZ, YZ
*   **4D Planes**: XW, YW, ZW

Each rotation step executes a standard 2D rotation matrix on the selected coordinate pair while preserving the remaining axes:

```javascript
// Rotate in the XW plane
let xNew = x * cos(aXW) - w * sin(aXW);
let wNew = x * sin(aXW) + w * cos(aXW);
x = xNew; w = wNew;
```

---

## 2. 4D-to-3D Hyper-Perspective Projection

To render a 4D tesseract onto a 2D viewport, the geometry must first project from 4D space down to 3D space, and then from 3D to 2D. 

The 4D-to-3D projection maps the $w$ coordinate using a perspective division factor:

$$x_{3d} = \frac{x_{4d}}{d - \frac{w}{r}}$$
$$y_{3d} = \frac{y_{4d}}{d - \frac{w}{r}}$$
$$z_{3d} = \frac{z_{4d}}{d - \frac{w}{r}}$$

Where:
*   $d$ is the hyper-camera focal distance (e.g., $2.3$).
*   $w$ is the rotated 4D depth value.
*   $r$ is the local joint radius scale.

This is implemented directly inside our joint-drawing pipeline:
```javascript
let factor = 1 / (2.3 - rotated.w / (radius * 0.65 + 0.01));
let p3dLocal = { x: rotated.x * factor, y: rotated.y * factor, z: rotated.z * factor };
```

---

## 3. Depth Sorting & Painter's Algorithm

Hugi's rasterization guides recommend sorting polygon faces from back-to-front (descending depth) to ensure proper occlusion sorting:
1.  Calculate the 3D projected coordinates for all 16 vertices.
2.  Aggregate the 24 cyclic faces.
3.  Compute the average 3D depth for each face:
    $$Depth_{avg} = \frac{p_{1}.depth + p_{2}.depth + p_{3}.depth + p_{4}.depth}{4}$$
4.  Sort descending and draw faces using alpha-blended strokes and fills:
```javascript
validFaces.sort((a, b) => b.depth - a.depth);
```
