# Elektor/Elektuur Issue #3: Clocks, Class A Amplifiers, & Telemetry

This document provides a detailed technical analysis of the engineering and physical designs presented in *Elektor* Issue #3 (March 1975) and early *Elektuur* archives, establishing how these models define clock triggers, sequencing, and power stages in the **TSFi2 Synthesis Studio**.

---

## 1. The "Mostronome" Electronic Metronome

Unlike traditional mechanical clocks or simple square-wave click generators, the **Mostronome** featured a unique whistle-like timing tick. It utilized discrete CMOS logic gates coupled with transistor shaping networks to generate a damped envelope oscillation.

```
                    Mostronome Resonant Tick Generator
                    
                  CMOS Gate                 Transistor Envelope
                  [Clock Trigger] ───┐       [RC Decay & Pitch]
                                     ▼              ▲
                                   ┌───┐            │
                  White Noise ────►│   │────────────┘
                  (Avalanche)      └───┘
                                     │
                                     ▼
                               Resonant Output
```

### A. Core Physics of the Tick
The metronome's click is created by triggering a high-Q bandpass resonator with a very short impulse. In the Mostronome:
1.  **impulse Gate**: CMOS logic gates (N1/N2) form a monostable multivibrator that outputs a brief gating pulse of width:
    $$t_w \approx 0.7 \cdot R \cdot C$$
2.  **Transistor Resonator**: The pulse kicks a high-Q twin-T or bridged-T active bandpass filter (similar to our Issue #2 drum engines) tuned to a whistling frequency ($f_{tick} \approx 800\text{Hz} - 1200\text{Hz}$).
3.  **Acoustic Shape**: The decay is fast, producing a crisp, clean woody "whack" sound rather than a sharp digital pulse.

### B. Synthesis Studio Sequencer Integration
In our Synthesis Studio, we can model this metronome trigger inside the ZMM VM. When a step sequencer increments, we can trigger a **Mostronome Tick** in Yul:
*   Instead of a digital click, the sequencer outputs a decaying sinusoidal tick at $1000\text{Hz}$ to keep tempo in multi-network simulations:
    $$V_{tick}(t) = E_{pulse} \cdot e^{-\alpha t} \sin(\omega_{tick} t)$$

---

## 2. The 10-Watt Class A Power Amplifier

Issue #3 presented a highly stable 10-Watt Class A amplifier design. The defining engineering challenge of Class A topologies is their high thermal dissipation and vulnerability to short circuits. 

### A. Constant-Current Source Protection
The Elektor design bypassed heavy output relays by using a **Constant-Current Source (CCS) Limiter** (using transistors Tr2–Tr5) to enforce absolute short-circuit protection:
*   The current source maintains a fixed bias current:
    $$I_{bias} = \frac{V_{ref}}{R_{bias}}$$
*   When a short circuit occurs, the current source restricts the maximum current passing through the output transistors, keeping them within their Safe Operating Area (SOAR) and preventing thermal runaway.

### B. Connection to the Equa-Amplifier Limiter
This constant-current protection aligns with our **Equa-Amplifier AGC** module:
*   Our Yul limiter (`equaAmplifier.yul`) enforces a dynamic voltage clamp ($V_{max} = I_{limit} \times R_L$) using the same physical current limits ($I_{limit} \approx 1.97\,\text{A}$).
*   By simulating the current source protection of the Class A amplifier, we ensure that no matter how loud the input modules are, the master output never clips or causes software overflow.

---

## 3. Automatic LCR Bridge Telemetry

The featured instrumentation project in Issue #3 was the **Automatic LCR Bridge**, which auto-ranged measurements of Inductance ($L$), Capacitance ($C$), and Resistance ($R$), outputting the values to LED digit displays.

*   **Closed-Loop Calibration**: We emulate this auto-ranging in our front-end Nixie tube telemetry system. The studio's Nixie indicator automatically scales its display window based on the active module:
    *   *Philicorda/Push-Pull*: Maps outputs to standard $1.0\text{V}$ ranges.
    *   *Germanium Stage*: Auto-ranges around the $7.5\text{V}$ DC offset.
    *   *Gyrator*: Ranges dynamically with the filter's resonant gain.
*   This mimics the LCR bridge's closed-loop calibration to ensure visual telemetry always operates in its optimal resolution window.
