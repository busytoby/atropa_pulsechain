# 📚 Elektuur Issue #25 (November 1964) Technical Analysis

This document details the simulation models and implementation strategy for two classic circuits from the debut November 1964 issue of *Elektuur* (Issue #25): the **Atoom-versterker (Atom Amplifier)** and the **Wien-Bridge RC Audio Generator**.

---

## 1. The "Atoom-versterker" (Atom Amplifier)

The *Atoom-versterker* is a classic Germanium-based Class-AB push-pull audio amplifier stage designed to deliver roughly $1\text{ W}$ of output power into a low-impedance ($8\ \Omega$) loudspeaker.

### 1.1 Schematic & Component Model
The circuit utilizes:
* **Pre-amplifier stage:** A single-ended common-emitter amplifier using a Germanium PNP transistor (e.g., OC71 or AC125).
* **Driver stage:** Transformer-coupled or direct-coupled phase splitter driving the output stage.
* **Output stage:** A matched pair of Germanium PNP power transistors (e.g., OC72 or AC128) operating in Class-AB push-pull configuration.
* **Crossover Bias:** A temperature-dependent biasing resistor (or early NTC thermistor) to prevent thermal runaway of the Germanium transistors while minimizing crossover distortion.

```
                    +Vcc (9V)
                      |
                     [R1] Bias
                      |
        Input ---[C1]--+--[Base Q1]
                           |
                     [Emitter Q1]--[R2]-- GND
                           |
                     [Collector Q1]
                           |
                         [ T1 Driver Transformer ]
                         /         \
                 [Base Q2]         [Base Q3]
                     |                 |
                   (Q2 PNP)          (Q3 PNP)
                     |                 |
                     +-------[ T2 ]----+
                             [Output]
                                |
                             [Speaker 8 Ohm]
                                |
                               GND
```

### 1.2 Mathematical Simulation Model
Germanium transistors exhibit a much lower base-emitter threshold voltage ($V_{be} \approx 0.2\text{ V}$) compared to silicon ($V_{be} \approx 0.7\text{ V}$), but suffer from high collector cutoff leakage current ($I_{cbo}$) that increases exponentially with temperature:

$$I_{cbo}(T) = I_{cbo}(T_0) \cdot 2^{\frac{T - T_0}{10}}$$

We model the collector current $I_c$ using the Ebers-Moll relation adjusted for Germanium leakage:

$$I_c = I_{s} \left( e^{\frac{V_{be}}{V_t}} - 1 \right) + I_{cbo}$$

The push-pull output voltage $V_{out}$ across the output transformer secondary impedance $R_L$ is:

$$V_{out}(t) = \eta \cdot (I_{c,Q2}(t) - I_{c,Q3}(t)) \cdot R_L$$

where $\eta$ is the turns ratio of the output transformer.

---

## 2. Wien-Bridge RC Audio Generator

The Wien-Bridge RC Generator is a classic test-bench oscillator designed to generate low-distortion sine waves from $20\text{ Hz}$ to $20\text{ kHz}$.

### 2.1 Wien-Bridge Network
The frequency-determining feedback network consists of a series RC arm and a parallel RC arm:

```
              ---[ R ]---[ C ]---+--- Output
                                 |
                                [R]
                                 |
                                [C]
                                 |
                                GND
```

The transfer function $\beta(\omega)$ is given by:

$$\beta(\omega) = \frac{V_{out}}{V_{in}} = \frac{1}{3 + j\left( \frac{\omega}{\omega_0} - \frac{\omega_0}{\omega} \right)}$$

where $\omega_0 = \frac{1}{RC}$. At resonance ($\omega = \omega_0$), the phase shift is $0^\circ$ and the attenuation is exactly $\frac{1}{3}$.

### 2.2 Amplitude Stabilization
To maintain a stable sine wave without clipping, the amplifier loop gain must be precisely $A = 3$. Elektuur solved this using a small incandescent bulb or NTC thermistor in the negative feedback loop. As the oscillation amplitude increases, the current through the NTC heating element increases, lowering its resistance and reducing the gain:

$$A(V_{RMS}) = 1 + \frac{R_f}{R_{NTC}(V_{RMS})}$$

We model the NTC resistance dynamics as:

$$\tau \frac{d\theta}{dt} = P_{diss}(t) - \delta(\theta - \theta_{amb})$$

$$R_{NTC}(\theta) = R_0 \cdot e^{B\left(\frac{1}{\theta} - \frac{1}{\theta_0}\right)}$$

---

## 3. Implementation Strategy in C/Yul Pipeline

We will implement a physical simulator for these two circuits in `tsfi_elektuur_issue25.c`:
1. **Atoom-versterker Simulation**:
   - Model the input signal amplification.
   - Simulate Germanium thermal run-away ($I_{cbo}$) by calculating temperature sweeps.
   - Output simulated distortion metrics (THD) mapping crossover effects when bias is misaligned.
2. **Wien-Bridge Oscillator**:
   - Implement a discrete-time ODE solver (Euler/Runge-Kutta 4th order) for the RC node voltages.
   - Model the NTC thermal time constant $\tau$ and output amplitude stabilization curves.

A test runner [test_elektuur_issue25.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_elektuur_issue25.c) will be created to verify these physical constraints under variable simulated temperatures.
