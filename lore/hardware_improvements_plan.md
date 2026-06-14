# TSFi2 Hardware Simulation Architecture Improvements Plan

This document proposes physical and numerical improvements to the ammeter, distributed coil, and triode valve models to enhance physical accuracy, numerical stability, and simulation performance.

---

## 1. Hot-Wire Ammeter Improvements

### A. Non-Linear Radiation Cooling (Stefan-Boltzmann Law)
*   **Current Model**: Cooling is modeled as purely linear convective heat dissipation:
    $$P_{\text{cooling}} = k_{\text{loss}} \cdot (T - T_{\text{ambient}})$$
*   **Improvement**: Introduce radiative cooling modeled by the Stefan-Boltzmann law for high-temperature heater wires:
    $$P_{\text{cooling}} = k_{\text{conv}} \cdot (T - T_{\text{ambient}}) + \epsilon \cdot \sigma_{\text{SB}} \cdot A_{\text{wire}} \cdot \left(T^4 - T_{\text{ambient}}^4\right)$$
    where $\epsilon$ is the platinum emissivity and $\sigma_{\text{SB}}$ is the Stefan-Boltzmann constant.

### B. High-Order ODE Integration (Runge-Kutta 4th Order)
*   **Current Model**: Simple Euler integration ($y(t+dt) = y(t) + dy \cdot dt$).
*   **Improvement**: Implement Runge-Kutta 4 (RK4) for the second-order needle mechanics to prevent numerical divergence when stiffness (high $K_{\text{spring}}$) or fast transient transients occur.

---

## 2. Distributed Coil & FDTD Grid Improvements

### A. Mutual Coupling between Multiple Coils
*   **Current Model**: Single coil FDTD grid mapping.
*   **Improvement**: Expand the 3D Yee grid mapping to accommodate multiple coils (e.g., tickler coil regenerative feedback or cascade transformer coupling) by defining overlapping coordinate volumes and mutual inductance parameters.

### B. AVX-512 Vectorization of Yee Loops
*   **Current Model**: Triple-nested sequential loops.
*   **Improvement**: Vectorize the E and H field tensor updates using AVX-512 intrinsics, aligning memory boundaries to 64 bytes to leverage high-throughput hardware execution.

---

## 3. Audion Valve & Grid Loading Improvements

### A. Non-Linear Grid Current & Loading
*   **Current Model**: Grid current is assumed negligible or zero.
*   **Improvement**: Model grid current ($I_g$) when the grid is driven positive ($V_g > 0$), resulting in physical loading (damping) of the input LC tank circuit:
    $$I_g = K_g \cdot \left(V_g + \frac{V_p}{\mu_g}\right)^{1.5}$$

### B. Dynamic Secondary Emission (Dynatron Effect)
*   **Current Model**: child's space-charge equation holds monotonically.
*   **Improvement**: Model secondary emission at high plate voltages where electrons hitting the plate liberate secondary electrons, creating negative resistance characteristics.
