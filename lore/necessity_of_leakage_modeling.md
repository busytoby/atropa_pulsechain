# The Architectural Necessity of Leakage Modeling in Physical Synthesis

This document analyzes why capacitive leakage modeling is not just a stylistic choice, but a **mathematical and operational requirement** for the physical simulation models inside the **TSFi2 Synthesis Studio**.

---

## 1. Why We Need Leakage: Module Case Studies

### A. The Philicorda Neon Relaxation Oscillator
The Philicorda neon oscillator uses a resistor-capacitor (RC) timing circuit:
* **The Physics**: The capacitor charges up toward $V_{cc}$ through a resistor. Once it hits the neon lamp's ignition voltage ($V_{ignite}$), the lamp conducts, acting as a low-resistance shunt that rapidly discharges (leaks) the capacitor down to the extinction voltage ($V_{extinct}$).
* **Without Leakage**: The capacitor would charge up once, hit the ignition threshold, and remain stuck at that voltage forever. No oscillation would occur, resulting in absolute silence.

### B. The Bionika Leaky Integrate-and-Fire (LIF) Neuron
The Bionika biological neuron models cell membrane ion currents:
* **The Physics**: When a stimulus is received, the membrane potential ($V_m$) rises. In the absence of a stimulus, the potential leaks back toward the resting potential ($0\text{V}$) through potassium leak channels.
* **Without Leakage**: Any background noise or input transients would accumulate indefinitely inside the cell, causing the neuron to fire continuous, runaway action potentials (epileptic loop state) regardless of whether input stimulus has ceased.

### C. The Diode Envelope Detector
The envelope tracker extracts signal amplitude envelopes:
* **The Physics**: A capacitor charges quickly through a rectifier diode on input peaks, and discharges slowly (leaks) through a parallel load resistor during signal valleys.
* **Without Leakage**: The envelope output would capture the absolute peak of the first audio wave and hold it forever, disabling any dynamic volume compression or companding.

---

## 2. Summary: Leakage is the Time Constant of Change

In analog systems, **leakage is the physical mechanism that enables decay, relaxation, and reset**. Without mathematically modeling charge dissipation (leakage paths):
1. **Dynamic range controls** lock up at their peak bounds.
2. **Oscillators** freeze at saturation limits.
3. **Generalization** in physical networks collapses into permanent bias lock.
