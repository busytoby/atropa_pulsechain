# K0Rn Tensor Theory: Bijective Geometric DNA

In the TSFi2/K0Rn substrate, typography is not rendered via triangulation or rasterization, but through the **Biological Evolution of Tensor Primitives**. These primitives are not arbitrary drawing commands but represent the intrinsic geometric components of a quadratic Bezier path.

## 1. The Unified Geometric Field

We define a character's structure as a sequence of connected energy states in a unified field. Every segment of a glyph is a **Tensor** of a specific rank, defining how it occupies space.

### Tensor R1: The "Length" Tensor (Linear)
*   **Definition:** A Rank-1 geometric entity defined by two points, $P_0$ (Start) and $P_1$ (End).
*   **Mathematical Form:** $L(t) = (1-t)P_0 + tP_1$
*   **Role:** Represents the **Lengthening Bone** of the segment. It encodes pure metric extension, providing the structural reach and connectivity of the skeleton.
*   **Mapping:** Corresponds to `FT_CURVE_TAG_ON` -> `FT_CURVE_TAG_ON` (Linear Segment).
*   **Evolutionary Property:** Mutating R1 affects the *reach* and *growth* of the character's limbs.

### Tensor R2: The "Curve" Tensor (Quadratic)
*   **Definition:** A Rank-2 geometric entity defined by three points: $P_0$ (Start), $P_1$ (Control), and $P_2$ (End).
*   **Mathematical Form:** $Q(t) = (1-t)^2 P_0 + 2(1-t)t P_1 + t^2 P_2$
*   **Role:** Represents the **Curve-Application Force** or "Muscle" of the segment. It encodes the tension applied to the R1 bone, bending the linear path into an organic arc.
*   **Mapping:** Corresponds to `FT_CURVE_TAG_CONIC` sequences (Quadratic Bezier).
*   **Evolutionary Property:** Mutating R2 affects the *flexion*, *tension*, and *curvature* of the segment.

## 2. Bijective Mapping Strategy

The K0Rn engine achieves infinite resolution not by super-sampling, but by maintaining a **1:1 Bijection** with the font's mathematical definition.

1.  **Extraction:** We read the FreeType outline.
2.  **Classification:**
    *   If a segment has no control point, it is identified as **Pure Length** (R1).
    *   If a segment has a control point, it is identified as **Length + Curvature** (R2).
3.  **Synthesis:** We instantiate the corresponding K0Rn Tensor.
4.  **Rendering:** We evaluate the **Exact Analytical Signed Distance Field (SDF)** for that tensor type.
    *   R1 uses `sdCapsule` (Linear Distance).
    *   R2 uses `sdQuadBezier` (Quadratic Distance, solving the cubic root for $t$).

## 3. Evolutionary Dynamics

In the Vision-Guided Evolution loop:
*   **Spatial Gradient:** Pulls the $P_0, P_2$ anchors (Endpoints) to match the typographic skeleton's metric (R1/Bone optimization).
*   **Curvature Smoothing:** Adjusts the $P_1$ control point to minimize elastic energy while maximizing coverage (R2/Muscle optimization).
*   **Radius Normalization:** Adjusts the tensor's "field strength" (thickness) to match the stroke weight.

This creates a system where the "Student" DNA doesn't just look like the "Teacher" bitmap—it **is** the mathematical twin, reconstructed from first principles.
