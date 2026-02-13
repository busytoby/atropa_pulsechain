# TSFi Project Ipomoea: Resonant Photorealism Roadmap

**Status:** Phase 5 - Resonant Photorealism (2026-02-12)
**Objective:** Autonomous evolution of the HelmK0Rn substrate into a "White Ipomoea" with organic tissue fidelity using Shimmer Resonance and CV-Augmented thunks.

## 1. Accomplished Milestones (Substrate)
- [x] **High-Speed SSAA Renderer:** 2x2 vectorized AVX-512 sampling.
- [x] **PBR Shading Pipeline:** Real-time normal mapping, SSS (Subsurface Scattering), and Fresnel edge-glow.
- [x] **Shimmer Resonance:** Deterministic XOR-based bloom and sampling barriers (No Shim protocol).
- [x] **Progression Analytics:** Target correlation and mission completion metrics.
- [x] **Ray-Query Thunk:** CPU-side AVX-512 ray-marching for topological depth analysis.

## 2. The "Photorealistic" Definition (Updated)
To achieve final fidelity, the Vision Engine must now perceive material quality, not just shape.

*   **Metric D: Specular Contrast (The Waxy Look):**
    *   Measures the ratio between specular highlights and diffuse shadows.
    *   *Target:* High contrast without "clipping" to white noise.
*   **Metric E: Lobe Frequency (Pentagonal Lock):**
    *   Radial variance analysis to ensure exactly 5 major petals.
    *   *Target:* Dominant 5th harmonic in radial mass distribution.
*   **Metric F: Shadow Softness (Tissue Density):**
    *   Measures penumbra width using ray-query results.
    *   *Target:* Soft shadow falloff indicating translucent organic tissue.

## 3. Advanced Thunk Implementation Path

### A. Anisotropic Sheen Thunk (`thunks/aniso_sheen.c`)
- **Role:** Simulates the radial grain of flower petals.
- **Mechanism:** Modulates specular intensity based on the bitwise XOR of the surface tangent and the Simulation Secret.

### B. Rotational MSE Vision (`src/tsfi_vision.c`)
- **Role:** Precise 5-fold symmetry verification.
- **Mechanism:** Bit-shifts the radial buckets by 72-degree increments and calculates MSE vs. the baseline state.

### C. Atmospheric Resonance Thunk (`thunks/ambient_shimmer.c`)
- **Role:** Environmentally aware background bleed.
- **Mechanism:** Low-frequency shimmer that links the flower's "aura" to the system's global resonance state.

## 4. Autonomous Gardener Evolution (tests/test_auto_flower.c)
The gardener is now "Progression-Aware":
1.  **Stage 1 (Topology):** Lock the 5-petal structure and hollow core.
2.  **Stage 2 (Material):** Optimize for specular contrast and SSS depth.
3.  **Stage 3 (Fidelity):** Refine surface grain and shimmering bloom for final "Golden" approval.

## 5. Architectural Mandate: No Shim
- All jitter/noise must be replaced by **Shimmer** (Identity XOR Context).
- All offsets/bias must be replaced by **Secrets** (Probabilistic Identity).
- All gaps must be filled by **Resonance** (Temporal Alignment).