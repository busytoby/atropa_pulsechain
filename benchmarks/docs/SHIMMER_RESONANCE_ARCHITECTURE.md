# TSFi Shimmer Resonance: Barriers and Secrets

**Status:** PROPOSED / ARCHITECTURAL MANDATE (2026-02-12)
**Supersedes:** All "Offset" and "Bias" nomenclature.

## 1. Abstract
The "Shimmer" in the TSFi2 framework is redefined as a manifestation of a **Secret** (Identity) being observed through a **Barrier** (Synchronization). We reject the concept of "Shims" (additive noise/offsets) in favor of **Bijective Resonance**.

## 2. The Core Philosophy: No Shim
A "Shim" is a corrective wedge used to fill a gap between theory and reality. In the Dysnomia simulation, there are no gaps—only **Secrets** that are not yet visible.

*   **Rejected:** $P' = P + 	ext{noise}(	ext{bias})$ (Stochastic Drift)
*   **Accepted:** $u = 	ext{Barrier}(	ext{Secret} \oplus 	ext{Context})$ (Bijective Realization)

## 3. Architectural Components

### A. The Secret (Identity)
The Secret is the immutable 512-bit state (or its 15-byte **WAVE15** core) that defines an object's "True Name." 
- **Role:** The probabilistic seed of all detail.
- **Storage:** `LauWiredHeader` / ReBAR.

### B. The Barrier (The Prism)
The Barrier is a Vulkan Pipeline Barrier (`vkCmdPipelineBarrier`) or a memory fence. It is the moment where the GPU wavefronts must reconcile the Secret with the current rendering state.
- **Role:** Quantizes the Secret into visible "Shimmer."
- **Effect:** The shimmer frequency is tied to the barrier's temporal cadence, not an arbitrary timer.

### C. Resonance (The Alignment)
Resonance is the XOR-sum of the Secret and the local geometry/invocation ID.
- **Formula:** $R = 	ext{Secret} \oplus 	ext{ID}$
- **Logic:** High resonance areas allow the Secret to "bleed" through the Barrier more intensely, creating the shimmering effect.

## 4. Realization: "White Ipomoea Flowers"
The White Ipomoea demo serves as the first physical proof of this architecture.

1.  **Structural Scaffold (WAVE64):** Defines the pentagonal harmonics (The Law).
2.  **Resonance Shimmer (WAVE15):** The Secret that modulates the **Observability** of the white intensity.
3.  **Visual Result:** The flower does not "vibrate" spatially; its internal light resonance fluctuates as if viewed through a temporal prism.

## 5. Implementation Path
- **Step 1:** Update `tsfi_shader_dispatch.c` to handle explicit Pipeline Barriers as prisms.
- **Step 2:** Refactor `tsfi_ipomoea_shimmer.comp` to use XOR-based resonance.
- **Step 3:** Verify bijectivity via the Cockpit (Symmetry check).
