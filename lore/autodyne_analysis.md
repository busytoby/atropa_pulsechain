# TSFi2 Autodyne Sounding Performance Analysis

This report evaluates the autodyne receiver simulation of the TSFi2 thermionic valve simulator using the parameters defined in `tests/test_autodyne_demodulation.c`.

## 1. Parameters and Simulation Setup

*   **RF Carrier Frequency ($f_{\text{incoming}}$):** 20,000 Hz
*   **Sampling Rate ($f_s$):** 96,000 Hz
*   **Valves Geometry:** Type T Tubular (cylindrical) triode configuration (`is_tubular = 1`, resulting in a `geom_scale = 0.85`).
*   **Valve Coefficients:**
    *   $\mu$ (Amplification factor) = $120.0$
    *   $K$ (Perveance) = $0.00003$
    *   $V_p$ (Plate supply voltage) = $250.0\text{ V}$
    *   $V_{g\text{ bias}}$ (Grid bias voltage) = $-1.5\text{ V}$
    *   $R_{\text{plate}}$ (Plate resistance) = $100,000\text{ \Omega}$
    *   $\beta$ (Autodyne positive feedback factor) = $0.90$
*   **Observed Output:** Peak beat tone of $2625\text{ Hz}$ with a Fourier magnitude of $0.010778$.

### Autodyne Local Oscillation
The positive feedback factor $\beta = 0.90$ drives the triode into self-oscillation at a local frequency $f_{\text{local}}$. Since the demodulated peak beat tone is at $2625\text{ Hz}$, the local oscillator frequency is:
$$f_{\text{local}} = f_{\text{incoming}} \pm f_{\text{beat}} = 20000 \pm 2625 \text{ Hz}$$
This indicates a local self-oscillation at either **$17,375\text{ Hz}$** or **$22,625\text{ Hz}$**.

---

## 2. Audio Quality and Harmonic Balance

### Warm Even-Order vs. Harsh Odd-Order Balance
1.  **3/2-Power Law Non-Linearity:** The triode model is based on Child's Law:
    $$I_p = K \left( \frac{V_p}{\mu} + V_g \right)^{1.5}$$
    The non-integer exponent ($1.5$) generates a smooth, dense series of harmonics. Unlike a symmetrical clipper (which creates only odd harmonics), the asymmetric nature of the triode transfer curve creates dominant even-order harmonics (especially the 2nd harmonic), resulting in a classic "warm" tube sound.
2.  **Dielectric Modulation:** The grid modulation model incorporates variable permittivity:
    $$\epsilon = 1.0 + \eta V_g$$
    With $\eta = 0.2$, the grid voltage is compressed asymmetrically. This dynamic compression further accentuates even-order harmonics and introduces second-order intermodulation products.
3.  **Tubular Geometry Dampening:** The cylindrical triode configuration applies a scaling factor of $0.85$ to the grid signal. This slightly dampens the grid swing, preventing harsh clipping/saturation (which would generate high-order odd harmonics and digital clipping).
4.  **Mixing Products:** Because the system operates as a mixer (autodyne detector), the output spectrum contains sum and difference frequencies of the carrier and local oscillator, along with their harmonics:
    $$n f_{\text{incoming}} \pm m f_{\text{local}}$$
    This produces a highly complex, vintage "heterodyne" sonic texture, combining a clear low-frequency beat with warm, tube-colored sidebands.

---

## 3. Proposed Architectural Improvements

To enhance the simulator and integrate it into a real-time synthesizer orchestration system, we propose the following improvements:

### A. Dynamic Ionization Modeling
*   **Current State:** The soft-vacuum ionization model uses the static supply voltage `vp_supply` to calculate the surge current.
*   **Improvement:** Make the ionization surge dependent on the **instantaneous plate voltage** $v_p(t)$. When the plate swings close to the ionization threshold $V_{\text{ionization}} = 30\text{ V}$, it should dynamically trigger gas discharge. This creates amplitude-dependent soft-clipping and realistic high-frequency "glow" grit.

### B. Oversampling and Anti-Aliasing
*   **Current State:** A 20 kHz RF carrier simulated at 96 kHz is highly prone to aliasing due to high-order harmonics folding back over the 48 kHz Nyquist limit.
*   **Improvement:** Implement internal $2\times$ or $4\times$ oversampling within the valve processor stage. Downsample back to the target sample rate using a linear-phase decimation filter to eliminate digital foldback distortion.

### C. Vectorized Regenerative Processing (AVX-512)
*   **Current State:** `tsfi_valve_process_regenerative` is a scalar loop because the regenerative feedback is sample-by-sample (`prev_vp = vp` from the previous sample).
*   **Improvement:** Introduce a 1-sample or 1-block delay in the feedback path with sub-sample interpolation. This allows processing in block batches of 16 using the AVX-512 vector execution pipelines (`tsfi_valve_process_avx512`), significantly reducing CPU overhead.

### D. Grid Leak Resistor-Capacitor (RC) Model
*   **Current State:** Feedback is directly fed to the grid.
*   **Improvement:** Add a simulated grid-leak capacitor and grid resistor network. The accumulation of grid current will cause the grid bias to shift dynamically based on signal level, producing organic compression, ducking, and "sag" under heavy drive.
