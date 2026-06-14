# DDJ Early 3D Graphics & TSFi2 Perspective Projection Engine

This document explores early 3D graphics algorithms published in **Dr. Dobb's Journal** (specifically Michael Abrash's work in the *Graphics Programming Black Book*), and demonstrates how to integrate these concepts into the **TSFi2** simulation engine to achieve hardware-efficient perspective projection and coordinate transforms.

---

## 1. Key 3D Concepts & Architectural Integration

| DDJ 3D graphics / Abrash Concept | Technical Implementation | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Perspective Divide** | Mapping 3D world space coordinates $(X, Y, Z)$ to 2D screen space using a scale factor divided by depth: $x' = (x \cdot f)/z$. | **Graphics Projection Module** | High-performance, overflow-safe Yul fixed-point division routines. |
| **$1/z$ Plane Interpolation** | Linear interpolation of reciprocal depth ($1/z$) across polygons to ensure perspective-correct texture mapping and depth ordering. | **Vector Rasterizer Pipeline** | Interpolation helper contracts in `graphicsSystem.yul` that calculate raster slopes. |
| **Polygon Clipping** | Sutherland-Hodgman clipping against viewport boundaries to avoid rendering off-screen coordinate overflows. | **Boundary Assertion Suite** | Signed comparisons and bounds check assertions inside coordinates plotting pipelines. |

---

## 2. Perspective Projection Mathematics

The goal of perspective projection is to simulate how three-dimensional objects appear smaller as their distance from the viewer ($Z$) increases. The equations mapping a 3D coordinate $(X, Y, Z)$ to 2D screen coordinates $(X', Y')$ are:
$$X' = \frac{X \cdot F}{Z} + C_x$$
$$Y' = \frac{Y \cdot F}{Z} + C_y$$
Where:
- $F$ is the focal length or distance to the projection plane.
- $(C_x, C_y)$ is the center offset of the viewport (screen center coordinates).

In resource-constrained environments (like our Yul VM), floating-point hardware division is unavailable, requiring signed 256-bit fixed-point math (scaled by $10^{18}$).

### Yul perspective Projection Library

Here is the implementation of perspective projection utilizing fixed-point division:

```yul
// TSFi2 3D Perspective Projection Library in Yul
// x, y, z are signed fixed-point coordinates (1e18 scale)
// focalLength is the projection plane distance (1e18 scale)
// centerX, centerY represent the viewport origin offsets
function project3DTo2D(x, y, z, focalLength, centerX, centerY) -> screenX, screenY {
    // Avoid division-by-zero or negative Z depth (behind camera clipping)
    if iszero(sgt(z, 0)) {
        // Return default offscreen coordinate if z is invalid
        screenX := 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
        screenY := 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
        leave
    }
    
    // Scale factor = focalLength / z (using 1e18 fixed-point division)
    let scaleFactor := sdiv(mul(focalLength, 1000000000000000000), z)
    
    // Project X: screenX = (x * scaleFactor) + centerX
    let rawProjX := sdiv(mul(x, scaleFactor), 1000000000000000000)
    screenX := add(rawProjX, centerX)
    
    // Project Y: screenY = (y * scaleFactor) + centerY
    let rawProjY := sdiv(mul(y, scaleFactor), 1000000000000000000)
    screenY := add(rawProjY, centerY)
}
```

---

## 3. Perspective-Correct $1/z$ Depth Buffer

Abrash introduced the method of interpolating $1/z$ across a scanline because $1/z$ is linear in screen space, whereas $Z$ is not. This allows graphics engines to perform correct texture mapping and depth buffer testing with simple linear additions:
1. At the vertices of a triangle, calculate $1/z$.
2. Linearly interpolate $1/z$ as you step across the polygon screen coordinates.
3. To retrieve actual depth for comparison, take the reciprocal of the interpolated value.

This optimization avoids calculating a full division at every pixel, replacing it with a single addition per step, which we implement in our rendering thunks to achieve low-latency screen updates.

---

## 4. Conclusion

By integrating Michael Abrash's perspective projection math and $1/z$ interpolation concepts, the TSFi2 graphics engine can render 3D structures and coordinate sweeps with minimal memory storage. We will incorporate these fixed-point functions directly into `graphicsSystem.yul` to power interactive 3D visualizers in the Synthesis Studio.
