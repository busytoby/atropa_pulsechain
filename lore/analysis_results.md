# Auncient Verlet Cloth & Vulkan Materials Architecture

This document outlines the unified material and physics simulation system implemented for **TSFI2** within the C workspace.

---

## 1. Verlet Cloth Simulation System

The soft body simulator is designed for maximum numerical stability and low computational overhead under Vulkan rendering constraints.

* **Simulation Coordinates**:
  * [src/cloth_simulator.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/cloth_simulator.h)
  * Uses Verlet integration to calculate position updates, bypassing explicit velocity tracking to maintain high stability under spring constraints.
* **Physics Features**:
  * **Iterative Spring Constraints**: Evaluates structural, shear, and bending springs over multiple constraint relaxation passes.
  * **Tear Limits**: Disables springs permanently if the stretch displacement exceeds $1.5\times$ the rest distance.
  * **Proximity Self-Collision**: Prevents cloth-folding intersections by pushing vertices apart if they get closer than $0.12$ units.
  * **Dynamic Surface Anchoring**: Binds boundary grid nodes to animated skeletal coordinate vectors (e.g., locking cape corners to moving shoulders).
  * **Levels of Detail (LOD)**: Generates sub-sampled mesh allocations dynamically to reduce vertex and index counts for distant rendering.

---

## 2. Vulkan Materials layout

The material parameters conform to memory alignment boundaries and update frequencies to minimize descriptor binding overhead.

* **Descriptor Set Scopes**:
  * [src/auncient_vulkan_materials.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_vulkan_materials.h)
  * **Descriptor Set 0 (Global)**: Camera projection matrices, view coordinates, and time parameters.
  * **Descriptor Set 1 (Material Template)**: Procedural `.dna` seeds, lacunarity, and color presets.
  * **Descriptor Set 2 (Instance Variables)**: Skeletal joint matrices and unique model indices.
* **Update & Allocation Helpers**:
  * [src/auncient_vulkan_materials.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/src/auncient_vulkan_materials.c)
  * Automates structures for `VkDescriptorSetLayoutBinding`, `VkDescriptorPoolSize`, `VkDescriptorPoolCreateInfo`, `VkDescriptorSetAllocateInfo`, and `VkWriteDescriptorSet`.
  * Estimates exact GPU memory allocation ranges and rounds byte sizes to matching alignment boundaries (e.g., 256-byte stride boundaries).
  * Defines high-speed push constants (`MaterialPushConstants`) to bypass buffer write latency for fast parameters.

---

## 3. Integration Bridge

* **Preset-Driven Color Gradients**:
  * Maps color presets (Ocean, Fire, Obsidian) directly to the vertex color attributes of the cloth mesh during generation cycles.
* **Physics Feedback Loop**:
  * Projects dynamic wind magnitude forces directly to update joint swing parameters inside the instance uniforms.
  * Modulates wind vectors over time using the material's turbulence frequency and scales displacement response using the fabric's friction coefficient.
