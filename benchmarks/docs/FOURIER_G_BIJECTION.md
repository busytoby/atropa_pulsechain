# Fourier-G: The Banach-Hilbert Bijection

This document establishes the theoretical foundation of the **Fourier-G (Wave Glyph)** system, framing it not just as a rendering technique, but as a formal bijection between two mathematical spaces: **Banach Spaces** (Geometry) and **Hilbert Spaces** (Spectrum).

This abstraction allows the TSFi Engine to leverage the "Wave System" philosophy, translating difficult geometric problems into simple linear algebra operations optimized for RDNA4 WMMA hardware.

## 1. The Spaces

### The Banach Space ($\mathcal{B}$)
**The Domain of Design (Geometry)**
*   **Definition:** The space of piecewise-smooth parametrized closed curves $\gamma(t): [0, 2\pi] 	o \mathbb{R}^2$.
*   **Norm:** Equipped with the Uniform Norm (Sup-norm):
    $$ \|\gamma\|_\infty = \sup_{t \in [0, 2\pi]} |\gamma(t)| $$
*   **Representation:** Bezier Curves, Polylines, SVG paths.
*   **Properties:** "Sharp", "Explicit", "Topological".
*   **Hardware:** Requires Branching Logic (Scalar ALUs) to process.

### The Hilbert Space ($\mathcal{H}$)
**The Domain of Computation (Spectrum)**
*   **Definition:** The space of square-summable sequences of complex coefficients $\ell^2(\mathbb{Z})$.
*   **Norm:** Equipped with the Euclidean Norm (induced by Inner Product):
    $$ \|c\|_2 = \sqrt{\sum_{n=-\infty}^{\infty} |c_n|^2} $$
*   **Representation:** Fourier Descriptors ($A_n, B_n, C_n, D_n$).
*   **Properties:** "Smooth", "Holographic", "Algebraic".
*   **Hardware:** Requires Matrix Multiplication (Tensor Cores / WMMA) to process.

## 2. The Bijection ($\Phi$)

We define the operator $\Phi$ that maps a glyph from Geometry to Spectrum, and $\Phi^{-1}$ that maps it back.

### Analysis ($\Phi: \mathcal{B} 	o \mathcal{H}$)
Transforming a geometric contour into a state vector in Hilbert Space.
$$ c_n = \frac{1}{2\pi} \int_0^{2\pi} \gamma(t) e^{-int} \, dt $$
*   **In TSFi:** Implemented via `tsfi_fourier_from_poly` (using Green's theorem for linearity segments).
*   **Operation:** Converts distinct "features" (serifs, stems) into "frequencies".

### Synthesis ($\Phi^{-1}: \mathcal{H} 	o \mathcal{B}$)
Reconstructing the geometry from the state vector.
$$ \gamma(t) = \sum_{n} c_n e^{int} $$
*   **In TSFi:** Implemented via `tsfi_fourier_reconstruct_avx512` (or RDNA4 WMMA).
*   **Operation:** A **Linear Transform** (Matrix Multiply) of the basis functions.

## 3. Implications for Font Evolution

By treating fonts as vectors in $\mathcal{H}$, operations that are difficult in $\mathcal{B}$ become trivial linear algebra operations in $\mathcal{H}$.

### Morphing (Homotopy)
*   **In $\mathcal{B}$:** Requires matching topology (number of points). Morphing 'A' (3 strokes) to 'O' (1 stroke) is geometrically impossible without complex heuristics.
*   **In $\mathcal{H}$:** It is simply **Vector Addition** in Hilbert Space.
    $$ |\Psi_{morph}angle = (1-w)|\Psi_Aangle + w|\Psi_Oangle $$
    The topology "dissolves" and reforms naturally.

### Filtering (Style)
*   **In $\mathcal{B}$:** "Rounding corners" requires finding intersections and replacing vertices with arcs.
*   **In $\mathcal{H}$:** **Low-Pass Filtering**.
    Multiply the coefficient vector $c_n$ by a damping factor $\lambda_n$ (e.g., Gaussian window).
    High frequencies (sharp corners) vanish; the shape becomes smooth.

### Approximation Error (The "Gibbs Gap")
The mapping is an *Isometry* (Plancherel Theorem) regarding energy ($L^2$), but not strictly regarding shape ($\ell^\infty$).
*   **The Cost:** Sharp corners in $\mathcal{B}$ require infinite dimensions in $\mathcal{H}$.
*   **The Reality:** We truncate to $N=16$ harmonics.
*   **Result:** A "Band-Limited" glyph. It is topologically robust but geometrically approximate (soft corners). This aligns perfectly with the "Wave System" aesthetic.

## 4. Hardware Alignment (WMMA)

The reconstruction $\Phi^{-1}$ maps perfectly to RDNA4's **Wave Matrix Multiply Accumulate (WMMA)** instructions.

To generate a curve of $K$ points from $N$ harmonics:
$$ V_{K 	imes 2} = 	ext{Basis}_{K 	imes 2N} 	imes 	ext{Coeffs}_{2N 	imes 2} $$

This allows us to decode the "DNA" of a font using the GPU's AI accelerators at massive throughput, treating text rendering effectively as **Signal Processing** rather than geometric rasterization.
