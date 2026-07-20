# TSFi2 Synthesis Studio: Elektor DSP Module Documentation

This document provides detailed documentation for the suite of low-level Yul virtual hardware DSP modules implemented in **TSFi2 Synthesis Studio** (derived from the *Elektor/Elektuur* archives, Issues #7 through #18). These modules form the active feedback and protection layers of the isolated trunk bypass bridge.

---

## Implemented Modules & Mathematical Models

| Issue | Module | Yul Object | Mathematical / Physical Model | Primary Selector |
| :--- | :--- | :--- | :--- | :--- |
| **#7** | **Rhythm Sequencer** | `RhythmGenerator` | 16-step dynamic clock triggers, pattern masks | `0x11223344` |
| **#8** | **Compander Dynamics** | `Compander` | Dual Compressor ($y = x^{0.5}$) / Expander ($y = x^2$) curves | `0x11223344` / `0x07a96d8c` |
| **#9** | **Hysteresis Limiter** | `HysteresisLimiter` | Sylvania gas-discharge hysteretic loop ($V_{\text{strike}} = 8\text{V}$, $V_{\text{ext}} = 5\text{V}$) | `0x11223344` / `0x07a96d8c` |
| **#10** | **Piezoresistive Limiter** | `PiezoLimiter` | Raytheon semiconductor pressure-modulated soft-clipping | `0x11223344` / `0x07a96d8c` |
| **#11** | **Zener Noise Dither** | `ZenerDither` | Reverse-biased silicon diode avalanche noise injection | `0x11223344` / `0x07a96d8c` |
| **#12** | **Sub-Harmonic Divider** | `SubharmonicDivider` | Philicorda octave divider energy redistribution ($f/2$) | `0x11223344` / `0x07a96d8c` |
| **#13** | **Wien-Bridge Notch Trap** | `WienBridgeNotch` | Resonant dynamic notch: $f_0 = \frac{1}{2\pi \sqrt{R_1 C_1 R_2 C_2}}$ | `0x11223344` / `0x07a96d8c` |
| **#14** | **Bionika Spiking Reflex** | `BionikaReflex` | Integrates overloads, fires stapedius reflex gain reduction | `0x11223344` / `0x07a96d8c` |
| **#15** | **Tape Saturation Shaper** | `TapeSaturation` | Telcan magnetic head cubic saturation: $y = x(1 - 0.25x^2)$ | `0x11223344` / `0x07a96d8c` |
| **#16** | **Thermal Half-Life** | `ThermalHalfLife` | Virtual junction temp drift ($dT/dt$) + Yttrium-90 release | `0x11223344` / `0x07a96d8c` |
| **#17** | **Axonal Delay Queue** | `AxonalDelay` | 12-sample FIFO delay buffer axonal conduction simulation | `0x11223344` / `0x07a96d8c` |
| **#18** | **Vactrol Opto-Compressor**| `VactrolLimiter` | Gallium Phosphide LED photocoupler organic compression | `0x11223344` / `0x07a96d8c` |

---

## Architectural Signal Flow

```
                     [ INPUT SIGNAL ]
                            |
                            v
               +------------+------------+
               |      PiezoLimiter       | <-- Pressure Modulated Soft-Clipping
               +------------+------------+
                            |
                            v
               +------------+------------+
               |     TapeSaturation      | <-- Telcan Hysteresis Soft-Saturator
               +------------+------------+
                            |
                            v
               +------------+------------+
               |     WienBridgeNotch     | <-- Dynamic Resonance Notch Filter
               +------------+------------+
                            |
                            v
               +------------+------------+
               |       ZenerDither       | <-- Limit-Cycle Noise Dispersion
               +------------+------------+
                            |
                            v
               +------------+------------+
               |       AxonalDelay       | <-- 12-Sample Axonal Conduction Buffer
               +------------+------------+
                            |
                            v
               +------------+------------+
               |      BionikaReflex      | <-- Stapedius Muscle Gain Attenuation
               +------------+------------+
                            |
                            v
               +------------+------------+
               |     ThermalHalfLife     | <-- Thermal Comp & Yttrium-90 Release
               +------------+------------+
                            |
                            v
               +------------+------------+
               |     VactrolLimiter      | <-- Optical Vactrol Knee Compression
               +------------+------------+
                            |
                            v
                     [ OUTPUT SIGNAL ]
```

### Wien-Bridge Selective Notch Filter
The Wien-Bridge Notch block uses a feedback network balanced at $f_0$. In Yul, it is parameterized via a dynamic cosine tracking coefficient $\cos(\theta_0)$ representing the center frequency:
$$y[n] = x[n] - 2\cos(\theta_0)x[n-1] + x[n-2] - a_1 y[n-1] - a_2 y[n-2]$$
where $a_1 = -1.9 \cos(\theta_0)$ and $a_2 = 0.9025$, yielding high-Q selective attenuation.

### Zener Avalanche Dither
Injects stochastic white noise directly into the filter loop state when clipping incidents occur, disrupting phase-locked limit cycles:
$$\text{Noise} \in [-10\text{mV}, +10\text{mV}]$$

### Bionika Reflex & Axonal Delay
Models leak integration, axonal conduction delay, and stapedius muscle protective clamping:
* **Axonal Delay**: 12-sample FIFO queue filters out transient clicks to prevent gain pumping.
* **Reflex potential**: $+300\text{mV}$ on overload.
* **Leak decay**: $80\%$ per sample step.
* **Firing threshold**: $800\text{mV}$.
* **Attenuation**: Drops gain factor to $0.7$ (refractory recovery phase decays gain back to $1.0$).

### Thermal Half-Life Protection
Models virtual junction temperature build-up based on power dissipation:
$$\tau \frac{dT}{dt} = \theta_{ja} P - (T - T_{ambient})$$
If $T > 320\text{K}$, active gain scaling applies $-1\%$ gain per degree K above $320\text{K}$. Release follows a Yttrium-90 $10\%$ recovery curve per step.

### Vactrol Opto-Compressor
Models Gallium Phosphide LED forward bias threshold ($1.8\text{V}$) driving a light-dependent photoresistor (LDR) shunt. Dynamic gain is evaluated as:
$$\text{Gain}_{opto} = \frac{R_{LDR}}{R_{series} + R_{LDR}}$$
providing slow, warm vactrol-knee compression.
