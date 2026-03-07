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

### The Mathematical Bridging Protocol
Because the `.dna` vessel must contain a complete, unified geometry for the PM4 C-Kernel to render, we use the two reliable anchor points the VLM provided to mathematically deduce the missing Phase 0 geometry:

*   **The Ear Coordinates:** Derived from the top 10% of the VLM's `Fur/Body Bounds` (`Y_min: 0.19`).
*   **The Snout Coordinates:** Anchored strictly beneath the VLM's `Eye Bounds` (`Y: 0.51`).
*   **The Limb Coordinates:** Spatially anchored to the bottom quadrants of the `Fur/Body Bounds` (`Y_max: 0.51` in the VLM's normalized output), stretching vertically to support the bipedal stand trigger.

DeepSeek-Coder is tasked with calculating these specific relative offsets to finalize the geometry before sealing the binary `.dna` file.

---

## Phase 0.5: Deep Structural Constraints & Environmental Math
A secondary, high-resolution VLM sweep was conducted to interrogate the exact physical materials, typography, and background environment of the generation. This data permanently governs how the Painter prompt and C-Kernel background are structured.

### 1. The Material Typography (Fur)
*   **VLM Observation:** *"The bear's body has a matted fur texture, which gives it a soft and furry appearance."*
*   **Architectural Mandate:** The Stable Diffusion Painter prompt must permanently include `matted fur texture` to satisfy the VLM's material baseline constraint.

### 2. The Sickness Mutation (The Missing Eye)
*   **VLM Observation:** *"The green sickness in the image is not a glowing aura... It is a teddy bear with a missing eye."*
*   **Architectural Mandate:** The VLM hallucinated a massive physical amputation. It did not perceive the sickness as a color overlay, but as a structural void. To satisfy the VLM logic, the C-Kernel must mathematically suppress or delete the Right Eye `TsfiTeddyEye` struct from the PM4 array during the sickness sequence, replacing it with a shadow primitive.

### 3. The Environmental Matrix (The Geometric Room)
*   **VLM Observation:** *"The background environment... consists of a geometric patterned wall, which features a combination of squares and rectangles. The lighting in the room is quite dim..."*
*   **Architectural Mandate:** The Wayland window cannot render a pure black void. The PM4 C-Kernel must calculate an array of massive background `PKT3_DRAW_RECT` primitives representing a dark, dim geometric grid to satisfy the VLM's spatial reasoning of the room.

---

## Phase 1: The Color & Aesthetic Matrix
Once the Phase 0 spatial bounds are locked, the VLM audits the target frame to establish the absolute color palette and environmental physics. During the Phase 1 interrogation, the VLM yielded the following structural directives:

### 1. The Material Typography (Fur)
*   **VLM Observation:** *"The bear has a mix of warm and cool tones in its fur... a matted fur texture."*
*   **Architectural Mandate:** The PM4 C-Kernel will calculate a base brown geometry, but the Stable Diffusion textual DNA must permanently enforce `variegated matted brown fur, mixed warm and cool tones` to achieve photorealism.

### 2. The Sickness Mutation
*   **VLM Observation:** *"A mix of both bright neon green and dark, rotting, earthy green."*
*   **Architectural Mandate:** The procedural noise function (`fract(sin())`) must map its output range. High noise peaks (`> 0.8`) draw `bright neon green`, while low-frequency sickness zones draw `dark earthy green`.

### 3. The Environmental Physics (Lighting)
*   **VLM Observation:** *"The light source casts a cool blue tone, which creates a blue-green hue in the image. The room is quite dim."*
*   **Architectural Mandate:** The Wayland window cannot render a pure black void. The PM4 C-Kernel must draw massive background `PKT3_DRAW_RECT` primitives representing a dark, geometric room. Furthermore, a semi-transparent `cool blue` specular highlight oval must be rendered on the top-left boundary of the Cranium to simulate the physical light source before the image hits the Painter.

### 4. The Lore Conflict (The Eye)
*   **VLM Observation:** *"The bear's eye is a solid color, which is a dark brown. There is no visible glowing gradient."*
*   **Architectural Resolution:** The VLM catastrophically hallucinated, prioritizing standard teddy bear anatomy over the Atropa/999 TSFi Lore. The foundational `.dna` explicitly states: `Baseline Eye is Green`. The C-Kernel is mathematically mandated to **override the VLM**, enforcing a strict `0x00FF00` (Kr0wZ Green) PM4 rendering for the locus of observation.

---

## Phase 2: Kinematic Vectors & Spatial Trajectory
To structurally dictate the 3-minute video timeline (specifically the 2:45 stand and walk-off), the VLM evaluated the spatial physics of the scene.

### 1. The Stand Amplitude
*   **VLM Observation:** *"If the bear were to abruptly stand up on its hind legs, approximately 50% of the total vertical frame would be occupied."*
*   **Architectural Mandate:** The C-Kernel kinematics must stretch the `body_radius_y` parameter by exactly 25% (doubling its sitting mass) during the 60-frame trigger sequence, while shifting the Cranium `g_y` coordinate upwards to match the new 50% frame occupancy.

### 2. The Walk Trajectory
*   **VLM Observation:** *"To completely walk off the left side of the stage... it must travel a horizontal line... making it appear as if the bear has been completely cut off from the scene."*
*   **Architectural Mandate:** The `g_x` vector must decrease linearly after the stand sequence. The bipedal stride is mathematically forced via a `sinf()` bobbing equation applied to the `g_y` axis, simulating the physical transfer of weight.

### 3. Dynamic Specular Lighting
*   **VLM Observation:** *"When the brown teddy bear stands up, its head is positioned above the light source in the top left corner... This means that the bear is closer to the light source."*
*   **Architectural Mandate:** The top-left specular highlight drawn by the C-Kernel must mathematically intensify in brightness/opacity as the `g_y` (Stand) variable increases. Moving closer to the light source structurally alters the rendering physics.