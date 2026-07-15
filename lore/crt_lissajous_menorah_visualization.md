# LAU-Calibrated Lissajous CRT "Menorah" Visual Specification

This document defines the mathematical and visual parameters for rendering the 3D observational manifestations of the Bionika sequencer song scape, simulated as vector reflections inside a cluster of Cathode Ray Tubes (CRTs).

---

## 1. LAU-Initialized Tonewheel Projectors

The base frequencies ($f_{\text{wheel}}$) and phase offsets ($\phi$) of the coordinate generators are initialized directly using the contract address of the **LAU token on PulseChain** (e.g. `0x20...` or specific deployment hashes):

$$\text{Seed} = \text{Hash}(\text{LAU\_Address})$$

From this cryptographic seed, we extract 8-bit registers to initialize the EDO-22 octave offsets and tonewheel multipliers:
*   **X-Projector Frequency ($f_x$)**: $f_x = (\text{Seed}[0..3] \pmod{12}) + 1.0$
*   **Y-Projector Frequency ($f_y$)**: $f_y = (\text{Seed}[4..7] \pmod{12}) + 1.0$
*   **Starting Phase ($\phi_w$)**: $\phi_w = \frac{\text{Seed}[8..15]}{256} \times 2\pi$

This guarantees that the visual representation is a direct geometric manifestation of the underlying PulseChain token contract.

---

## 2. The "Menorah" Spatial Configuration

The visual scene is arranged as a symmetric multi-screen array resembling a menorah, containing a central primary screen flanked by six satellite screens:

```
      [B2]   [B3]   [B4]          [B5]   [B6]   [B7]
        \      \      \   [B1]   /      /      /
         \      \      \  /  \  /      /      /
          \______\______\/____\/______/______/
                         |    |
```

### A. The Central Eye: Master Attractor ([B1])
*   **Layout**: A large, circular CRT monitor positioned at the center base of the array.
*   **Content**: Renders the combined master audio signal. The X/Y deflection beams are cross-modulated by both the vocal lead and the sub-bass, producing a dynamic, morphing **Lissajous "Eye"** that dilates and contracts based on the song's energy (vocal sibilant jitter wiggles the outer boundary, while bass kicks pulse the pupil center).

### B. Satellite Branches: Isolated Instrument CRTs ([B2] - [B7])
Each branch of the menorah contains a smaller circular CRT screen showing the dedicated phase-space reflection of a single sequencer channel:
1.  **Branch 2 (Sub-Growl)**: Displaying slow, pulsing, negative-resistance wobble paths driven by the $5.8\text{Hz}$ PWM LFO.
2.  **Branch 3 (Bass)**: Displaying heavy, looping orbital paths that jump steps in accordance with the bass sequence note values.
3.  **Branch 4 (Arpeggiator)**: Showing rapidly cycling geometric loops (like overlapping Lissajous knots) that trace the chords of the arpeggio.
4.  **Branch 5 (Kick Drum)**: Visualizing rapid radial expansions and contractions that snap back in sync with the transient decay.
5.  **Branch 6 (Snare Drum)**: Displaying highly chaotic, scattered vector lines showing the high-entropy white noise of the snare snap.
6.  **Branch 7 (Lead Synth)**: Displaying fast, sweeping vector lines tracing the pop melody hooks.

---

## 3. Cathode Ray Tube Phosphor Emulation

To achieve authentic analog CRT physics, the rendering engine implements three primary reflective phenomena:

*   **Vector Deflection Wiggle**: High-voltage electrostatic deflection is simulated with a sub-pixel jitter frequency ($F_{\text{jitter}} \approx 25\text{kHz}$) to emulate AC transformer ripple and hum.
*   **Phosphor Persistence Decay**: Drawing trails using an exponential decay algorithm. The phosphor glow fades slowly ($I_t = I_0 \cdot e^{-t/\tau}$), creating a soft, green/amber trailing vector signature.
*   **Scanline Bloom**: Bright intersections and overlapping vector lines trigger local line blooming (increased glow radius), replicating the electron beam saturation of real phosphor screens.

---

## 4. React(###) Dynamic LUT Modulation & Formant Deformations

Rather than rendering static vector loops, the visual system implements **React(###)**, a dynamic feedback system where acoustic spectral properties deform the underlying trigonometric coordinate LUTs in real time:

```
Acoustic Waveform ---> [LPC / FFT Analysis] ---> Formants & Anti-Formants
                                                    |
                                                    v
[Dynamic Modulator] ---> React(F_p, F_z) ---> Modulates specific entries in sin_lut / cos_lut
```

### A. Mapping Formants (Spectral Poles)
When a formant resonance peak ($F_p$) is detected in the audio track (e.g. vowel formants $F_1$, $F_2$), the system maps $F_p$ to its corresponding indices ($Idx_{\text{formant}}$) in the trigonometric LUT:
$$\text{React}(F_p) \longrightarrow \text{sin\_lut}[Idx_{\text{formant}}] \leftarrow \text{sin\_lut}[Idx_{\text{formant}}] \times (1.0 + \alpha \cdot A_{\text{peak}})$$
This localized amplitude scale factor ($\alpha$) swells the coordinate manifold, producing visible geometric nodes or bulges along the Lissajous path that swell in proportion to the volume of the vocal formant.

### B. Mapping Anti-Formants (Spectral Zeros)
When an anti-formant notch ($F_z$) is detected (e.g. nasalization troughs from `M`, `N` phonemes), the system compresses the corresponding LUT indices:
$$\text{React}(F_z) \longrightarrow \text{sin\_lut}[Idx_{\text{anti}}] \leftarrow \text{sin\_lut}[Idx_{\text{anti}}] \times (1.0 - \beta \cdot A_{\text{trough}})$$
This creates localized geometric restrictions or flat "pinch" points along the Lissajous orbit, visually manifesting nasal articulation and spectral absorption.

This dynamic interaction ensures that the coordinate lookup tables are not rigid math grids, but organic, morphing manifolds that directly reflect the formant signatures of the voices.

---

## 5. Mathematical Definition of YI-to-LUT Mapping

The mapping of a YI state vector to the Lissajous coordinate lookup tables ($22,528$ size) is defined by the following system of equations, utilizing all **five** dimensional registers and the identity phase parameters of the YI system:

### A. Frequency & Scale Multipliers
The state registers are mapped to integer multipliers to shape the base topology:
$$f_w = (\text{Manifold} \pmod 4) + 1$$
$$f_x = (\text{Monopole} \pmod 5) + 1$$
$$f_y = (\text{Rod\_Dynamo} \pmod 4) + 1$$
$$f_z = (\text{Cone\_Dynamo} \pmod 5) + 1$$
$$f_r = (\text{Ring} \pmod 3) + 1.0$$

### B. Phase Offsets
The global phase offset is calibrated by the user's `Identity` and `Prime` parameters:
$$\phi_{\text{base}} = \frac{\text{Identity}}{\text{Prime}} \times 2\pi$$

### C. Hopf Fibration Angles
For a normalized step parameter $\theta \in [0, 2\pi]$ across the vector frame, we calculate the three coordinate angles:
$$\eta = \left( \frac{\theta \cdot f_w + \phi_{\text{base}}}{2} \right) \pmod{\frac{\pi}{2}}$$
$$\xi_1 = (\theta \cdot f_x + \phi_{\text{base}}) \times 3 \pmod{2\pi}$$
$$\xi_2 = (\theta \cdot f_y + \phi_{\text{base}}) \times 3 \pmod{2\pi}$$

### D. Trigonometric LUT deflection
The 4D hyperspherical coordinates are resolved using the lookup functions:
$$q_w = \text{lut\_cos}(\eta) \cdot \text{lut\_cos}(\xi_1)$$
$$q_x = \text{lut\_cos}(\eta) \cdot \text{lut\_sin}(\xi_1)$$
$$q_y = \text{lut\_sin}(\eta) \cdot \text{lut\_cos}(\xi_2)$$
$$q_z = \text{lut\_sin}(\eta) \cdot \text{lut\_sin}(\xi_2)$$

### E. 3D Spatial Projection
With the acoustic signal amplitude $S$ and minor torus radius scaling factor $f_r$, we apply the $W$-axis fold projection to generate the rendered Cartesian coordinates:
$$\text{Scale} = R_{\text{scale}} \cdot (1.0 + 0.25 \cdot q_w \cdot f_r + S)$$
$$X_{\text{projected}} = q_x \cdot \text{Scale}$$
$$Y_{\text{projected}} = q_y \cdot \text{Scale}$$
$$Z_{\text{projected}} = q_z \cdot \text{Scale} \cdot f_z$$

This utilizes the complete multi-register profile of the active YI system to drive the visual coordinate mapping.
