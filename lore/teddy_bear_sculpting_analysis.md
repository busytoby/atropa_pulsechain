# Auncient Teddy Bear Sculpting Analysis & Geometry Evolution

This document details the mathematical structures and procedural improvements applied to the teddy bear mesh using the apEx systems.

## 1. Geometric Resolution and Topology
* **Base Primitive Alignment**:
  * Utilizes connected sphere templates representing the Torso, Cranium, Ears, Snout, Nose, Tail, and Limbs.
* **Subdivision Upgrades**:
  * Replaced the low-polygon 8-sector grids with a high-density 16-sector ring resolution, preventing angular facet boundaries during rendering.
* **Relative Coordinate Spacing**:
  * All coordinates are mapped relative to the Torso origin `(0.0, 0.0, 0.0)`.
  * The Cranium is positioned at `y = 1.8`, ears are mapped to `y = 2.4` and `x = ±0.6`, upper limbs sit at `y = 0.3` and `x = ±1.1`, and lower limbs sit at `y = -1.1` and `x = ±0.6`.

## 2. Anatomical Sculpting & Deform Modifiers
* **Snout and Nose Integration**:
  * Added a front snout ellipsoid at `y = 1.65` and `z = 0.7`, and a small circular nose tip at `y = 1.7` and `z = 0.95` to define the muzzle anatomy.
* **Sickness Mutation (The Eye Cavity)**:
  * Implemented a distance-weighted deform operator centered on the right eye coordinates `(0.3, 1.8, 0.7)`.
  * Vertices falling within a `0.4` radius are pushed inward along the Z-axis using a linear decay factor, creating a smooth physical cavity.

## 3. High-Frequency Surface Texturing
* **Lambertian Light Modulation**:
  * Implemented an orthographic light projection where surface intensity is mapped to normal vectors.
* **Procedural Fur Bump-Mapping**:
  * Modulates the lighting normals using a high-frequency sine/cosine wave pattern mapped to the UV texture coordinates.
  * This creates the visual effect of matted fur clumps without requiring mesh polygon increases, preserving the compact Auncient VM storage footprint.
