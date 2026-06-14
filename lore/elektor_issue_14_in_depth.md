# Elektor/Elektuur Issue #14: Bionika Spiking Neurons and Biological Acoustic Reflex Loops

This document provides a detailed technical analysis of the engineering and physical principles presented in *Elektor (Electronica Wereld)* Issue #14 (October 1963), focusing on how spiking neural circuits can model biological protective reflexes to handle clipping incidents in the **TSFi2 Synthesis Studio**.

---

## 1. Bionika Threshold Spiking Neuron

Issue #14 introduces **Bionika**, the study of biological systems replicated via electrical circuits. It details the discrete simulation of a neuron's axon membrane potential using neon bulbs, capacitors, and transistors to model leak integration, threshold firing, and refractory cooling.

### A. The Neural Potential Equation
The membrane potential $V_m(t)$ integrates incoming stimulus current $I_{stim}(t)$ with a leak resistance $R_{leak}$:

$$\tau_m \frac{dV_m}{dt} = -V_m + I_{stim}(t) R_{leak}$$

* When $V_m(t) \ge V_{threshold}$, the neuron fires a sharp output spike: $S(t) = 1.0$.
* Immediately after firing, the membrane potential is reset to $0$, and the neuron enters a **refractory period** ($\tau_{ref}$) during which it cannot fire, establishing a frequency limit.

---

## 2. Application: Neural Acoustic Reflex (Stapedius Muscle Emulation)

In the human auditory system, loud acoustic transients trigger the **acoustic reflex** (contraction of the stapedius muscle), which mechanically stiffens the ossicular chain, reducing sound transmission to the cochlea to prevent damage.

By modeling this biological reflex using the Bionika Neuron:
1. **Overload Integration**: Every time a sharp clipping incident occurs in the Synthesis Studio (e.g., $V > 0.95 \cdot V_{max}$), we inject a high-current stimulus pulse into the virtual Bionika neuron.
2. **Threshold Spiking**: If clipping is transient, the neuron leaks it away. If clipping occurs repeatedly in a short window, the neuron's potential breaches the threshold and fires.
3. **Global Attenuation Reflex**: The neuron's output spike triggers an immediate, aggressive global gain attenuation (reducing `masterAttenuation` by $30\%$) and increases formant filter damping ($Q$) for $150\text{ms}$.
4. **Refractory Cooldown**: The system slowly releases the attenuation as the neuron recovers, restoring linear operation cleanly.
