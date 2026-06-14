# Elektor/Elektuur Issue #4: Loops, Compression, & Converters

This document analyzes the technical and mathematical concepts introduced in *Elektor* Issue #4 (April 1975) and Elektuur archives, mapping these principles directly to frequency modulation, level stabilization, and visual diagnostics in the **TSFi2 Synthesis Studio**.

---

## 1. Phase-Locked Loop (PLL) Systems

*Elektor* Issue #4 detailed the operational physics of **Phase-Locked Loops (PLLs)**, particularly focusing on the NE565/CD4046 IC architectures. In analog synthesis, PLLs are used for frequency tracking, ring modulation, and multiplying/dividing VCO frequencies.

```
                         Phase-Locked Loop (PLL) Architecture
                         
                  Input       ┌───────────────┐
                  Signal ────►│ Phase Detector│───────┐
                  (Ref)       └───────▲───────┘       │ Error
                                      │               ▼ Voltage (V_e)
                              ┌───────┴───────┐   ┌───┴──────────┐
                              │  Oscillator   │◄──│ Loop Filter  │
                              │     (VCO)     │   │  (Low Pass)  │
                              └───────────────┘   └──────────────┘
```

### A. Closed-Loop Phase Correction Math
A PLL is a negative feedback control system consisting of a phase detector, a low-pass loop filter, and a voltage-controlled oscillator (VCO). The phase detector compares the input phase ($\theta_{in}$) with the VCO phase ($\theta_{vco}$), generating an error voltage ($v_e$):
$$v_e(t) = K_d \sin(\theta_{in}(t) - \theta_{vco}(t))$$
Where $K_d$ is the phase detector gain. The loop filter integrates this error voltage to output a control voltage ($v_c$) that adjusts the VCO frequency:
$$\frac{d\theta_{vco}}{dt} = \omega_0 + K_o v_c(t)$$
This closed-loop system locks the VCO to the input frequency, allowing the synthesizer to follow external audio pitch or track complex waveforms with mathematical precision.

---

## 2. Analog Feedback Compressor

The **Compressor** project in Issue #4 focused on dynamic range compression using a feedback control loop.

*   **Gain Reduction**: An input signal is amplified, and its peak envelope is tracked. Whenever the envelope exceeds a threshold, it regulates a shunt element (such as a photo-resistor in an optocoupler or a variable-conductance transistor base) to reduce input gain:
    $$G(t) = \frac{G_0}{1 + k \cdot V_{env}(t)}$$
*   **Studio Connection**: This feedback gain reduction expands on our **Equa-Amplifier AGC**. While the Equa-Amplifier acts as a protective peak limiter, a feedback compressor provides longer attack/release decay times (e.g. $10\text{ms} - 200\text{ms}$), which are crucial to sustain and smooth out percussive signals (like our Snare and Kick drums).

---

## 3. LED-Level Comparator Ladder

The **LED-Level** project detailed a multi-stage voltage level indicator, which uses a ladder of comparative op-amps to turn on LED segments sequentially as voltage increases.

*   **Comparator Physics**: The input signal $V_{in}$ is compared against a series of resistive division steps:
    $$V_{ref\_n} = V_{ref} \times \frac{n}{N}$$
    If $V_{in} \ge V_{ref\_n}$, the $n$-th comparator activates, lighting the LED.
*   **Decade Visualization**: This comparator ladder logic matches our virtual **Z550M Nixie indicator**. By translating continuous signal amplitudes into discrete threshold steps, we represent dynamic voltage sweeps numerically across the 0–9 neon indicators.

---

## 4. Integration Roadmap for the Studio

To expand the studio using Issue #4:
1.  **Develop a PLL Ring Modulator**: Implement a Yul module (`pllModulator.yul`) that locks onto an input pitch and outputs locked sub-harmonics for ring modulation.
2.  **Add Feedback Compressor controls**: Extend `equaAmplifier.yul` to support variable attack and release coefficients in its storage envelope slot, allowing users to toggle between hard limiting and soft compression.
