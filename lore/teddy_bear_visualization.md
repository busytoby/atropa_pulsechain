# TSFi Atropa/999: Teddy Bear Visualization Architecture

## The Tri-Model AOT Compilation Matrix
The generation of the Sovereign Choreography relies on an Ahead-Of-Time (AOT) neural pipeline designed to bypass Python/ML latency ceilings. We utilize three distinct models to forge a single `.dna` binary vessel, which is then streamed natively to Wayland via PM4 integers.

1. **Stable Diffusion (The Painter):** Generates the initial photorealistic reference of the Sick Teddy Bear based on lore.
2. **Moondream2 (The VLM):** Audits the painting and extracts structural logic.
3. **DeepSeek-Coder (The Architect):** Translates the VLM logic into strict C-struct mathematical boundaries.

---

## Phase 0: The Primary Offset Matrix
The defining architectural law of the TSFi pipeline is that we must **get all the offsets first**. 

Before a single pixel is painted by Stable Diffusion or critiqued by the VLM, the absolute mathematical boundaries of the entity must be mapped and frozen. If the geometry is allowed to float, the neural models will experience catastrophic anatomical drift.

We execute this by establishing a rigid hierarchy of relative offsets tied to the main body anchor:
1.  **The Origin Anchor:** The center of the primary body mass `(X: 512, Y: 875)` becomes the absolute reference point for all other geometry.
2.  **Cranial Offset:** Calculated relative to the body mass (e.g. `Y - 475`).
3.  **Auditory Offsets (Ears):** Calculated relative to the Cranium (`Cranial_X ± 150`, `Cranial_Y - 150`).
4.  **Locus Offsets (Eyes):** Calculated relative to the Cranium (`Cranial_X ± 40`, `Cranial_Y`).

By mathematically locking this entire hierarchy of offsets *first*, we guarantee that when the bipedal stand and walk-cycle triggers, the entire entity moves as a single, structurally unified mass without the limbs detaching from the torso. The AI models are no longer guessing where the bear is; they are strictly obeying the Phase 0 Offset Matrix.

---

## The Moondream Hallucination & Bounding Box Discovery
During the initial AOT sequence, the Moondream Vision-Language Model was commanded to extract the precise RGB Hex codes and sickness percentages from the photorealistic reference frame. 

**The VLM Output:**
> *"The exact RGB hex color of its fur is `[0.19, 0.13, 0.51, 0.49]` and the exact RGB hex color of its eyes is `[0.26, 0.41, 0.51, 0.52]`..."*

### The Neural Accident
Moondream mathematically failed the color request. Instead of hex codes, the neural network collapsed into its foundational object-detection training matrix. It outputted strict normalized spatial bounding boxes in the format `[Y_min, X_min, Y_max, X_max]`.

### The Geometric Application
This hallucination is an architectural breakthrough. Rather than arbitrarily guessing the radii of the PM4 circles (`head_radius = 200`), we feed these VLM bounding boxes directly into DeepSeek to mathematically derive the exact anatomical proportions for the Phase 0 Offsets.

*   **Fur/Body Bounds:** `[0.19, 0.13, 0.51, 0.49]`
    *   The body natively occupies the frame between 13% and 49% on the X-axis, and 19% to 51% on the Y-axis.
*   **Eye Bounds:** `[0.26, 0.41, 0.51, 0.52]`
    *   The eyes sit precisely between 41% and 52% on the horizontal plane.

## The Incomplete Anatomy & Missing Geometry
While the VLM successfully extracted the bounds for the primary mass (The Body) and the locus of observation (The Eyes), it fundamentally failed to map the complete biological structure of the bear (omitting Ears, Muzzle, and Limbs).

When prompted to find these specific structures, the VLM's object-detection weights collapsed, returning the maximum image boundary `[0.0, 0.0, 0.99, 0.99]`. 

### The Secondary Interrogation (The Final Offsets)
To finalize the Phase 0 matrix, the VLM was explicitly re-interrogated with isolated geometric prompts to prevent attention-collapse. It successfully extracted the remaining architectural bounds:

1. **The Snout/Muzzle:** `[0.39, 0.44, 0.67, 0.74]`
    * The exact spatial bounds for the C-Kernel snout primitive. It sits mathematically centered, slightly lower than the eyes.
2. **The Spatial Lighting:** `Top-Left Origin`
    * The VLM confirmed the Painter generated a primary light source from the top-left corner. The C-Kernel must enforce this by adding a secondary, semi-transparent highlight layer on the upper-left boundary of the Cranium and Body radii.
3. **The Sickness Locus:** `Left Eye Cluster`
    * The noise generation should not be perfectly uniform. The mathematical focus of the Kr0wZ Green procedural noise must originate from and be thickest around the Left Eye coordinate constraint.
