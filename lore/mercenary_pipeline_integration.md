# Improving the Mercenary Virtual World Engine via the AOT Pipeline

This document details how we apply our **7-Phase Tri-Model Ahead-Of-Time (AOT) Compilation Pipeline** to upgrade the classic *Mercenary* 3D wireframe and room database engine, automating coordinate layout, physics bounding, and texturing.

---

## 1. Upgraded Compilation Matrix for Mercenary

```
[ Phase 1: SD (Reference) ] --> [ Phase 1.1: Multi-View Blueprints ]
                                                |
[ Phase 2.5: Kinematic & Hulls ] <-- [ Phase 2: VLM Object Discovery ]
             |
[ Phase 3: DeepSeek Compiler ]   --> [ Phase 4: Runtime Shader & Database ]
                                                |
                                     [ Phase 7: RAG Telemetry Loop ]
```

---

## 2. Phase-by-Phase Upgrades

### Phase 1 & 1.1: Asset Blueprinting (Stable Diffusion & Multi-View)
* **Operation**: Stable Diffusion generates 3D orthographic multi-view blueprints of the spaceships, room interiors, and building meshes (e.g. the central hangar or planetary bunkers).
* **Benefit**: Ensures clean vector projections. We extract the exact depth profiles ($Z$-axis) of rooms and spaceships instead of using hardcoded cuboids.

### Phase 2: Bounding Box Discovery (Moondream2 VLM)
* **Operation**: The VLM scans the multi-view blueprints and isolates structural coordinates:
  * **Room Boundaries**: Wall, ceiling, and floor dimensions.
  * **Interactivity Zones**: Locations of doors, keycard readers, and energy portals.
* **Benefit**: Automatically determines local $X, Y, Z$ positions, eliminating the need to manually program inventory coordinates.

### Phase 2.5: Collision Hull & Joint Rigging
* **Operation**: Wraps room objects and walls in collision primitives (infinite planes for floors/walls, bounding box hulls for items).
* **Joint Rigging**: Defines motion joints for opening doors, rotating spaceship wings, and elevator vertical slides.
* **Benefit**: Calculates collision bounding zones before writing code, allowing the Yul engine to check player position against walls using simple primitive inequalities instead of complex vertex distance checks.

### Phase 3: DeepSeek Database & Code Compilation
* **Operation**: Takes the rigged boundaries and automatically packs them into the 256-bit metadata slots (`2000000 + objId`) and generates the Yul rotation/projection routines.
* **Benefit**: Auto-generates the database mappings and updates target vertex tables (`$D7F0`–`$D8EF`) without manual register writing.

### Phase 4: Run-Time Engine (Soft Diffusion Textures)
* **Operation**: The runtime engine renders the projected 3D coordinates. It applies the **Soft Diffusion** noise function (`fract(sin())`) to add visual texture to:
  * The planetary ground grid (simulating Targ's rocky desert terrain).
  * The walls of the Target Building (simulating matted concrete paneling).
* **Benefit**: Achieves styled, detailed textures on a flat 3D wireframe grid with zero frame rate impact.

### Phase 7: RAG Ingestion & Self-Improvement Loop
* **Operation**: Records player movement trajectories, frame rates, and room transition speeds. Telemetry is saved back to `lore/telemetry_feedback.md`.
* **Benefit**: If a player gets stuck or experiences frame rate drops in highly complex rooms, the next compile cycle reads the telemetry and automatically simplifies the vertex count or adjusts boundary collision scaling.
