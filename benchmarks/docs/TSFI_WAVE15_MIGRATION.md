# TSFi WAVE15 Migration Protocol

**Objective:** Transition from static, compiler-enforced alignment (`__attribute__((aligned(N)))`) to dynamic, capacity-based alignment using the `WAVE15` atomic structure.

## The Strategy: Variadic WAVE Capacity

Instead of defining fixed-size structs with heavy padding, we treat all geometric and compute data as streams of `WAVE15` atoms (15 bytes).

*   **Atom:** `WAVE15` (15 bytes, packed).
*   **Container:** Allocated memory block (guaranteed aligned by `lau_malloc_wired` / `lau_memalign`).
*   **Alignment:** Handled by the *capacity* of the container, not the *type* of the element.

## Validated Boundaries

Our tests confirm the following capacity mappings for `WAVE15`:

| Hardware Boundary | Capacity (WAVE15s) | Used Bytes | Gap |
| :--- | :--- | :--- | :--- |
| 32 Bytes | 2 | 30 | 2 |
| 64 Bytes | 4 | 60 | 4 |
| 128 Bytes | 8 | 120 | 8 |
| 256 Bytes | 17 | 255 | 1 |
| 512 Bytes | 34 | 510 | 2 |

## Migration Steps

1.  **Strip `aligned()` Attributes:** Remove `__attribute__((aligned(X)))` from target structures in `inc/`.
2.  **Adopt `packed`:** Ensure structures are `__attribute__((packed))` to prevent compiler padding.
3.  **Update Allocators:** Use `lau_malloc_wired` or `lau_memalign` to ensure the *base pointer* is aligned (usually 512-byte for AVX-512 compatibility).
4.  **Refactor Access:** When accessing data using vector instructions (AVX-512), ensure we use:
    *   Unaligned loads/stores (`vmovdqu`, `_mm512_loadu_si512`).
    *   OR, strictly manage indices to ensure 64-byte alignment boundaries are respected where performance is critical.

## Target Structures for Cleanup

*   `TSFiGlyphRegistry` / `TSFiGlyphRegistryPacked` (inc/tsfi_font_vectors.h)
*   `TSFiPackedSegment` (inc/tsfi_font_vectors.h)
*   `TSFiHelmholtzOperator` / `TSFiFeynmanVoxel` (inc/tsfi_svdag.h)
*   `GeneticNode` (inc/tsfi_genetic.h) - *Careful review needed for AVX usage*

## Benefits

*   **Memory Efficiency:** Reduced padding waste.
*   **Flexibility:** "WAVE of Any Size" support.
*   **Code Cleanliness:** Removal of non-standard, compiler-specific attributes scattered throughout the headers.
