# Elektor/Elektuur Issue #2: Active Synthesis, Gyrators, & Percussion

This document analyzes the engineering and physical principles presented in *Elektor* Issue #2 (January/February 1975) and *Elektuur* Issue #2 (September 1961), detailing how they can be used to construct advanced filter networks and percussion units for the **TSFi2 Synthesis Studio**.

---

## 1. Inductor Simulation: The Active Gyrator

A key engineering challenge in early analog synthesizer design was the size, cost, and electromagnetic interference (hum pick-up) of physical inductors ($L$). *Elektor* Issue #2 introduced the **Gyrator**, an active circuit that simulates an inductor using an active gain element (transistors/op-amps), resistors, and a capacitor.

```
                  Active Gyrator Circuit (Simulated Inductor)
                  
                           R1
                  Vin o───▀▀▀▀───┬──────────────┐
                                 │              │
                                ▀▀▀ C           ▼  Active
                                ▀▀▀             ▲  Buffer (Gain=1)
                                 │              │
                                 ├──────────────┘
                                 │
                                ▀▀▀ R2
                                ▀▀▀
                                 │
                                ─── GND
```

### A. The Physics & Math of the Gyrator
The gyrator reverses (gyrates) the impedance of a capacitor. The equivalent inductance ($L_{eq}$) is determined by:
$$L_{eq} = R_1 \times R_2 \times C$$

For example, to simulate a large $1.0\text{H}$ inductor using a small $100\text{nF}$ capacitor:
*   Set $R_1 = 1\text{k}\Omega$ and $R_2 = 10\text{k}\Omega$.
*   $$L_{eq} = 1000 \times 10000 \times (100 \times 10^{-9}) = 1.0\text{H}$$

### B. Synthesizer Application: Formant Bandpass Filters
In the Synthesis Studio, we can implement a virtual **Gyrator Bandpass Filter**. By placing the simulated inductor in series with a resistor and capacitor, we build an active LC bandpass filter without winding coils. 
*   **Speech Synthesis (Bionika)**: Human vowels (formants) are represented by high-Q bandpass resonances (e.g., $F_1 \approx 500\text{Hz}$, $F_2 \approx 1500\text{Hz}$). The active gyrator filter allows us to create precise, dynamically sweepable formant resonators in Yul:
    $$y[n] = b_0 x[n] + b_1 x[n-1] + b_2 x[n-2] - a_1 y[n-1] - a_2 y[n-2]$$
    where the coefficients are computed directly from the simulated $L_{eq}$ and tuning resistances.

---

## 2. Drum & Percussion Synthesis

Issue #2 featured discrete transistor drum simulator circuits. These circuits generate transient percussion sounds (kick drums, tom-toms, and bongo clicks) by applying a trigger impulse to a highly resonant filter.

### A. Bridged-T Resonator
The core of early analog percussion is the **Bridged-T network** (or Twin-T network), which acts as a bandpass filter operating right on the edge of self-oscillation (high Q factor).

```
                      Bridged-T Percussion Resonator
                      
                              C1             C2
                  Trigger o───▀▀▀───┬────────▀▀▀───o Output
                                    │
                                    ▀ R1
                                    ▀
                                    │
                                   GND
                                 ┌──┴──┐
                                 │  R2 │  (Negative feedback loop
                                 └──┬──┘   to control Q decay)
                                    │
                                    ┴
```

1.  **Trigger Impulse**: A brief trigger pulse (modeled on the `BionikaNeuron` spike) kicks the network.
2.  **Damped Oscillation**: The resonator rings at its resonant frequency ($f_0 = \frac{1}{2\pi \sqrt{R_1 R_2 C_1 C_2}}$) and decays exponentially:
    $$V_{out}(t) = A \cdot e^{-\alpha t} \sin(\omega_0 t)$$
    *   **Decay rate ($\alpha$)**: Controlled by the negative feedback loop.
    *   **Pitch ($f_0$)**: Adjusted to emulate a kick drum ($50\text{Hz} - 80\text{Hz}$) or a tom-tom ($100\text{Hz} - 250\text{Hz}$).

---

## 3. Dynamic Bias Stabilization (Elektuur #2, Sept 1961)

In the early Germanium era (1961), temperature changes caused significant drift in collector current ($I_c$), leading to distortion and thermal runaway. Issue #2 detailed the engineering principles of **Dynamic Bias Stabilization**.

### A. Temperature Compensation Math
Germanium leakage current ($I_{cbo}$) doubles approximately every $8^\circ\text{C}$. To stabilize the operating point, emitter degeneration ($R_e$) and potential divider biasing ($R_1, R_2$) are used:

$$S = \frac{dI_c}{dI_{cbo}} = \frac{1 + \frac{R_b}{R_e}}{1 + \beta \frac{R_b}{R_e}}$$
Where $R_b = \frac{R_1 R_2}{R_1 + R_2}$. A low stability factor $S$ (close to 1) prevents drift.

### B. Studio Implementation
When simulating early Germanium common-emitter configurations in the ZMM VM, we implement:
*   **Dynamic base-current feedback loops** to scale the bias voltage dynamically.
*   An **emitter stabilization capacitor voltage offset** that tracks signal envelopes and automatically scales gain, mimicking real-world thermal and voltage stabilization.

---

## 4. Integration Roadmap for the Studio

To expand the **TSFi2 Synthesis Studio** using Issue #2:

1.  **Add a Gyrator Formant Filter Module**: Write a Yul contract (`gyratorFilter.yul`) implementing a sweepable active resonant filter.
2.  **Create a Drum Percussion Module**: Build a triggered bridged-T generator (`drumSynthesizer.yul`) to generate electronic drum hits.
3.  **UI Rack Extension**: Update `studio.html` to include controls for the filter resonance (Q-factor), simulated inductance (L), and drum trigger parameters.
