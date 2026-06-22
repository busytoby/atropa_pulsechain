# Auncient Procedural Physics & Genome Integration Report

This document records the mathematical integration of **Thomas Jakobsen's Verlet constraint framework** and **Joar Jakobsson's (TIGSource) procedural Slugcat/Scavenger physics** into the project's DNA compiler, C-based graphics thunks, and Z-machine instruction registers.

---

## 1. DNA Compiler Customization (Rain World Genome)

The DNA compiler in [compile_dna.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/compile_dna.py) has been rebuilt to inject Joar Jakobsson's procedural traits directly into the `atropa.dna` binary asset structure.

### Active Genomic Trait Mapping
* **`body_fatness` (segment width multiplier):** Scales target constraint distances and visual circle radii.
* **`muscle_stiffness` (Verlet relaxation decay):** Determines how fast physical twitches decay back to resting states.
* **`walk_frequency` (kinematic pacing):** Dictates step timing and tail wiggle cycles.

```python
GENOME = {
    "cranial_scale": 1.1,
    "body_fatness": 1.25,
    "tail_length": 0.95,
    "muscle_stiffness": 0.90,
    "walk_frequency": 0.60
}
```

---

## 2. Thunks & C graphics Integration

We replaced the static circle calculations in [test_wayland_teddy_standalone.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_teddy_standalone.c) with a complete C-based 4D Verlet particle solver using 13 particles and 12 constraints.

### The 4D Verlet Update (Implicit Velocity)
$$\vec{p}_{i, n+1} = \vec{p}_{i, n} + (\vec{p}_{i, n} - \vec{p}_{i, n-1}) + \vec{a} \cdot \Delta t^2$$

### Constraint Relaxation Loop
Runs $N$ iterations to project particle positions along constraint vectors, maintaining target lengths:
$$\vec{p}_2 \leftarrow \vec{p}_2 + \vec{d} \cdot \text{percent}$$
$$\vec{p}_1 \leftarrow \vec{p}_1 - \vec{d} \cdot \text{percent}$$

### Segment Capsule Rendering
Draws connected ellipses using a thick line segment projection algorithm (`draw_capsule`), mimicking the organic connected-shapes style of Slugcats.

---

## 3. Z-Machine Integration Blueprint

To tie these physics properties to the retro game engine, the Z-machine registers (defined in the Yul bytecode and JSON contracts) are mapped to trigger physical forces:

* **Z-Machine Object Attributes ($D68F$ / $D716$):**
  Poke triggers in the VM call Yul physics routines, transforming retro inputs (such as jumping or attacking) directly into Verlet momentum impulses ($\vec{a}$) in the C animation loop.
* **Tension-to-Audio Mapping:**
  Average constraint offsets are mapped to the TB303 synthesizer's cut-off frequency, causing the audio to morph dynamically based on limb strain.
