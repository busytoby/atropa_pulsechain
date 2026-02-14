# TSFi Phase 2: Project Ipomoea (Automated Vision-Guided Evolution)

**Objective:** Autonomous evolution of the K0Rn substrate into smooth floral and glyphic structures using real-time Computer Vision metrics and component-based geometric composition.

## 1. The "Ipomoea" Target Definition
To guide the genetic algorithm, the Vision Engine utilizes phenotype characteristics defined in `assets/flowers/*.flower` and explicit classifiers in `assets/classifiers/*.class`.

*   **Metric A: Radial Symmetry & Profile:**
    *   Captured via `tsfi_vision_analyze_staging` using 8-bin radial signature mapping.
    *   Compares real-time manifestations against "Gold Standard" profiles for species-level identification.
    *   *Target:* High correlation with species-specific profiles (IDs 10-16).

*   **Metric B: Topological Identity:**
    *   Detects closed loops (holes) using flood-fill analysis.
    *   Strict enforcement of `required_holes` differentiates glyphs (e.g., 'B' requires 2 holes) from floral substrate (0 holes).

*   **Metric C: Geometric Complexity:**
    *   Evaluates spatial variety and rich detail.
    *   *Target:* `target_coverage` (typically ~0.25) balanced with high-frequency surface grain (veins).

## 2. Realization Kernels & Thunks

The implementation utilizes dedicated AVX-512 thunks for real-time realization and self-awareness:

1.  **Component Rendering (`src/tsfi_k0rn_compiler.c`):**
    *   Generalized architecture using smooth arcs (multi-segment), axial stems, and spines.
    *   Eliminates "boxy" or square-based construction in favor of smooth, aesthetic curvatures.

2.  **Self-Classification (`MAPPED_COMMON_FIELDS`):**
    *   All mapped structures now include a `classify()` thunk.
    *   Enables objects to semantically identify themselves (e.g. `Xenostegia` vs `Latin_B`) even during genetic flux.
    *   JIT-wired automatically during allocation via the `lau_memory` system.

3.  **PBR Shading (`thunks/ipomoea_pbr.c`):**
    *   Applies multi-zone chromatic zonation (Core -> Body -> Outer).
    *   Calculates surface normals from the SDF stream for physical lighting (SSS, Fresnel, Diffuse).

## 3. The Universal Genetic Loop

The system supports universal recombination across all phenotype assets:

1.  **One K0Rn Mandate:** Exactly one original hardcoded K0Rn exists (the Teddy Bear). All other phenotypes (A, B, Ipomoea) are derived manifestations or loaded from serialized DNA assets (`.dna`).
2.  **Cross-Phenotype Splicing:** `tsfi_k0rn_crossover` supports mixing arbitrary phenotypes (e.g., crossing a 'B' with a 'Xenostegia') to generate hybrid life forms.
3.  **Semantic Morphing:** Real-time interpolation allows for "Smooth Transitions" where the `classify()` thunk tracks the precise frame where identity shifts from one class to another.

## 4. Execution Strategy

1.  **Verify Manifestation:** Run `run_all_flowers.sh` to manifest and verify the 7 core species IDs (10-16).
2.  **DNA Evolution:** Use `test_auto_scribe --glyph 0x42` to evolve and tune glyph DNA against topological targets.
3.  **Universal Verification:** Execute `tests/verify_all_flowers.c` to confirm that all manifests meet the "Gold Standard" radial and topological resonance.

## 5. Augmenting the Phenotype Collection

To add a new floral species to the Ipomoea collection, follow this codified process:

1.  **Register Class ID:** Add a new `TSFI_CLASS_*` entry to the `TSFiResonanceClass` enum in `inc/tsfi_vision.h`.
2.  **Define Phenotype:** Create a `.flower` file in `assets/flowers/` defining the ideal radial profile, colors, and geometric perturbation frequencies (trident/curling).
3.  **Define Classifier:** Create a `.class` file in `assets/classifiers/` matching the target class ID and defining the vision weights and thresholds.
4.  **Manifest & Evolve:** Run the Autonomous Gardener using `./tests/test_auto_flower --species <name> --test` to evolve the K0Rn substrate and save a "Gold Standard" checkpoint.
5.  **Verification:** Run `./run_all_flowers.sh` to ensure the new species is correctly identified and manifests within visual integrity bounds.
