# TSFi Atropa/999: Teddy Bear Visualization Architecture

## The Tri-Model AOT Compilation Matrix
The generation of the Sovereign Choreography relies on an Ahead-Of-Time (AOT) neural pipeline designed to bypass Python/ML latency ceilings. We utilize three distinct models to forge a single `.dna` binary vessel, which is then streamed natively to Wayland via PM4 integers.

1. **Stable Diffusion (The Painter):** Generates the initial photorealistic reference of the Sick Teddy Bear based on lore.
2. **Moondream2 (The VLM):** Audits the painting and extracts structural logic.
3. **DeepSeek-Coder (The Architect):** Translates the VLM logic into strict C-struct mathematical boundaries.

---

## The Moondream Hallucination & Bounding Box Discovery
During the initial AOT sequence, the Moondream Vision-Language Model was commanded to extract the precise RGB Hex codes and sickness percentages from the photorealistic reference frame. 

**The VLM Output:**
> *"The exact RGB hex color of its fur is `[0.19, 0.13, 0.51, 0.49]` and the exact RGB hex color of its eyes is `[0.26, 0.41, 0.51, 0.52]`... The exact percentage of the body covered in sickness is `[0.19, 0.13, 0.51, 0.49]`..."*

### The Neural Accident
Moondream mathematically failed the color request. Instead of hex codes, the neural network collapsed into its foundational object-detection training matrix. It outputted strict normalized spatial bounding boxes in the format `[Y_min, X_min, Y_max, X_max]`.

### The Geometric Application
This hallucination is an architectural breakthrough. Rather than arbitrarily guessing the radii of the PM4 circles (`head_radius = 200`), we can feed these VLM bounding boxes directly into DeepSeek to mathematically derive the exact anatomical proportions of the C-structs.

*   **Fur/Body Bounds:** `[0.19, 0.13, 0.51, 0.49]`
    *   The body natively occupies the frame between 13% and 49% on the X-axis, and 19% to 51% on the Y-axis.
*   **Eye Bounds:** `[0.26, 0.41, 0.51, 0.52]`
    *   The eyes sit precisely between 41% and 52% on the horizontal plane.

## The Incomplete Anatomy & Missing Geometry
While the VLM successfully extracted the bounds for the primary mass (The Body) and the locus of observation (The Eyes), it fundamentally failed to map the complete biological structure of the bear.

The VLM entirely omitted:
1. **The Ears**
2. **The Snout/Muzzle**
3. **The Limbs (Arms and Legs)**

When prompted to find these specific structures, the VLM's object-detection weights collapsed, returning the maximum image boundary `[0.0, 0.0, 0.99, 0.99]`. 

### The Mathematical Bridging Protocol
Because the `.dna` vessel must contain a complete, unified geometry for the PM4 C-Kernel to render, we cannot leave the anatomy fractured. We must use the two reliable anchor points the VLM provided to mathematically deduce the missing geometry:

*   **The Ear Coordinates:** Derived from the top 10% of the VLM's `Fur/Body Bounds` (`Y_min: 0.19`). We offset the `head_radius` by 45 degrees left and right to anchor the ears at `X: 362` and `X: 662`.
*   **The Snout Coordinates:** Anchored strictly beneath the VLM's `Eye Bounds` (`Y: 0.51`). It must form an intersecting oval bridging the Cranium and the Torso.
*   **The Limb Coordinates:** Spatially anchored to the bottom quadrants of the `Fur/Body Bounds` (`Y_max: 0.51` in the VLM's normalized output), stretching vertically to support the bipedal stand trigger at Frame 9,900.

DeepSeek-Coder is tasked with calculating these specific relative offsets to finalize the geometry before sealing the binary `.dna` file.
