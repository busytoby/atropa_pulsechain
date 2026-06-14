# Elektor/Elektuur Issue #17: Nerve Conduction Delay Lines, Tube Saturation, and Temporal Filtering

This document provides a detailed technical analysis of the engineering and physical principles presented in *Elektor (Elektronika Wereld)* Issue #17 (February 1964), focusing on how axonal nerve conduction delays and push-pull tube saturation help us prevent sharp clipping in the **TSFi2 Synthesis Studio**.

---

## 1. Axonal Nerve Conduction Delays (Zenuwgeleidingsonderzoek)

Issue #17 highlights research into **nerve conduction** (zenuwgeleidingsonderzoek). In biology, action potentials do not travel instantaneously; their velocity along the axon is governed by membrane resistance ($R_m$) and capacitance ($C_m$), creating a propagation delay:

$$v_{propagation} \propto \frac{1}{\sqrt{R_m C_m}}$$

### A. Application: Axonal Delay Line
We can model this propagation speed as a FIFO (First-In, First-Out) delay queue for our Bionika ear-reflex loop:
- **Axonal Latency**: When the Bionika neuron fires a threshold spike, the signal is written into a 12-sample propagation buffer (simulating a $12\text{ms}$ nerve conduction delay).
- **Temporal Filtering**: Short, single-sample click transients are absorbed or ignored by the delay before they can clamp the master gain.
- **Sustained Resonance Protection**: If clipping persists, successive spikes fill the queue, triggering the ear-reflex attenuation after the latency period.
- **Acoustic Result**: Prevents unnecessary gain pumping on short clicks, while remaining protective against sustained howling.

---

## 2. ECLL80 Combined Triode-Pentode Tube Compression

The issue reviews the **Lorenz ECLL80**, a unique tube containing a triode phase inverter and two output pentodes in a single envelope, used for high-fidelity push-pull power amplification.

### A. Phase-Aligned Symmetrical Saturation
Push-pull configurations cancel even-order harmonics but generate soft odd-order harmonic saturation. 
* By modeling the ECLL80's transfer characteristics inside our output stages, we can round off clipping waveforms symmetrically.
* **Math**: The push-pull output is represented by:
  $$V_{out} = V_p \cdot \tanh\left(\frac{V_{in}}{V_p}\right)$$
  which provides a smooth, soft-knee transition into clipping.
