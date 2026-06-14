# Elektor/Elektuur Issue #9: Stroboscopic Hysteresis, Avalanche Thresholds, and Sweep Synchronization

This document provides a detailed technical analysis of the engineering and conceptual frameworks presented in *Elektor (Electronica Wereld)* Issue #9 (December 1962), focusing on how these gas-discharge and threshold trigger designs help us optimize the anti-clipping systems of the non-robotic formant filters in the **TSFi2 Synthesis Studio**.

---

## 1. Stroboscopic Relaxation Hysteresis (Sylvania Trigger Generator)

Issue #9 detailed the **Sylvania Gas-Discharge Relaxation Oscillator** (stroboscope), which charges a capacitor until it reaches the strike threshold ($V_{strike} \approx 8\text{V}$) and discharges it rapidly until it falls below the extinguish threshold ($V_{extinguish} \approx 2\text{V}$).

### A. The Hysteretic Window Physics
The system state is bistable and governed by a hysteretic loop:

$$\text{State}(t) = \begin{cases} 
\text{Discharging} & \text{if } V_c \ge V_{strike} \text{ or } (V_c > V_{extinguish} \text{ and } \text{State}(t-dt) = \text{Discharging}) \\
\text{Charging} & \text{otherwise}
\end{cases}$$

### B. Application to Filter Protection engaged/disengaged logic
In standard clipping protection (like dynamic companding or Q-damping feedback), engaging and disengaging the protection at a single voltage threshold causes **gain chattering** (rapid switching), which produces harsh high-frequency distortion (zipper noise).

By applying stroboscopic hysteresis:
* **Strike Threshold**: The anti-clipping dampener or compander activates only when the signal peak exceeds a high strike voltage (e.g., $90\%$ of headroom).
* **Extinguish Threshold**: The protection remains active and releases slowly until the tracked envelope decays below a lower extinguish threshold (e.g., $40\%$ of headroom).
* **Acoustic Result**: This eliminates chattering distortion, ensuring smooth gain transitions and preserving the natural, warm formant resonances.

---

## 2. Avalanche Breakthrough Physics (Halfgeleider als Stralingsdetector)

The radiation detector article reviewed how solid-state junctions biased near their avalanche breakthrough threshold conduct heavily when triggered by incoming alpha particles or ionizing radiation.

### A. Telemetry Incident Loggers
In the Synthesis Studio, we model this avalanche conduction for our **Overload Clipping Telemetry**:
- **Avalanche Trigger**: The moment the filtered audio signal breaches the critical clipping boundary, it triggers a virtual "avalanche breakthrough" event.
- **Incident Logger**: This event immediately increments the collision counter and logs the telemetry incident on the dashboard, lighting up the Nixie indicator and displaying warning alerts instantly.

---

## 3. CRT Sweep Synchronization (Miniatuur TV-Ontvanger)

The miniature TV monocle design requires precise synchronization of horizontal and vertical sweep clocks to prevent image tearing during flyback phases.

### A. Zero-Crossing Protection Transitions
If the compander or Q-damping changes gain factors abruptly mid-wave, it introduces phase discontinuities that ruin the non-robotic qualities of vowel formants. 
By incorporating CRT sweep synchronization:
* We align gain updates (flyback transitions) with the **zero-crossings** of the stimulus waveform.
* This keeps the resonant filter phase sweeps coherent and smooth, preventing phase tearing and maintaining full signal transparency.
