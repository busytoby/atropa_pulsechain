# TSFi Phase 2: Project Ipomoea (Automated Vision-Guided Evolution)

**Objective:** Autonomous evolution of the HelmK0Rn voxel/SDF substrate into a "White Ipomoea" flower structure using advanced, real-time Computer Vision metrics (AVX-512).

## 1. The "Ipomoea" Target Definition
To guide the genetic algorithm, we must mathematically define the visual characteristics of an Ipomoea flower so the Vision Engine can "see" it.

*   **Metric A: Radial Symmetry (The Funnel):**
    *   Ipomoea flowers exhibit high rotational symmetry.
    *   *Implementation:* An AVX-512 kernel that samples pixels at `(x, y)` and compares them to pixels at `(-x, -y)` (180-degree) or rotated coordinates.
    *   *Target:* `Symmetry Score > 0.85`.

*   **Metric B: Radial Density Profile (The Throat):**
    *   Cross-section: An Ipomoea has a deep, empty "throat" (dark/low density at absolute center) surrounded by a blooming "limb" (high density ring).
    *   *Implementation:* A radial histogram kernel. It calculates the average brightness at Ring 0 (Center), Ring 1 (Mid), and Ring 2 (Rim).
    *   *Target:* `Center < Rim` (The "Void Center" topology).

*   **Metric C: Edge Smoothness (The Petals):**
    *   The rim must be continuous, not scattered noise (which our current Laplacian filter already helps with).
    *   *Target:* `Complexity < 0.2` (Smooth curves) AND `Coverage > 0.3` (Sufficient mass).

## 2. Advanced Vision Kernels (src/tsfi_vision.c)

We will implement two new high-speed kernels:

1.  `tsfi_vision_analyze_symmetry_avx512`:
    *   Folding the image buffer onto itself using SIMD permutations.
    *   Calculates the Mean Squared Error (MSE) between the left and right halves (or quadrants).

2.  `tsfi_vision_analyze_radial_avx512`:
    *   Uses a pre-computed distance lookup table (LUT) or fast approximation to bin pixels into "Center", "Mid", and "Rim" buckets during the scan.

## 3. The Autonomous Loop (tests/test_auto_flower.c)

We will create a specialized "Gardener" test harness:

1.  **Initialization:** Spawn a random population of K0Rn genomes (spheres/folds).
2.  **Render Cycle:** Render the genome to the framebuffer (headless or visible).
3.  **Vision Pass:** Run `Symmetry` + `Radial` analysis.
4.  **Decision Engine:**
    *   *Score = (Symmetry * 2.0) + (RadialFit * 1.5) + Smoothness*
    *   If `Score > BestScore`: Keep genome (Save to `best_flower.k0rn`).
    *   If `Score < BestScore`: Revert and Mutate.
5.  **Termination:** Stop when `Score > Threshold` (A recognizable flower exists).

## 4. Execution Strategy

1.  **Implement Kernels:** Upgrade `tsfi_vision.c` with symmetry/radial logic.
2.  **Verify Metrics:** Create a test that renders a known circle vs. noise and proves the metrics distinguish them.
3.  **Launch Gardener:** Run the automated evolution and watch the shape emerge from the chaos.
