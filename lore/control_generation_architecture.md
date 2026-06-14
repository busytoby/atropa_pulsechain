# Synthesis Studio: Control Generation Architecture & Standards

This document establishes the specifications and standards for utilizing the **TSFi2 Synthesis Studio** primarily as a **Control Generator** (generating Control Voltages (CV), low-frequency modulation (LFOs), gates, clock dividers, and neural spike triggers) to drive downstream systems, processes, and agent behaviors.

---

## 1. Shift from Audio to Control Paradigm

Traditional synthesizer designs apply high-pass filtering (AC-coupling) to remove DC offsets. In a **Control Generation** architecture, DC offsets are the primary carrier of information (representing steady-state control values, slider positions, or bias voltages).

### Key Architectural Shifts:
1.  **DC-Coupled Signal Paths**: All Yul modules must maintain a DC-coupled output. High-pass filtering is disabled to preserve steady-state control voltages ($0.0\text{V} - 10.0\text{V}$).
2.  **Low-Frequency Optimization (LFO/CV)**: Oscillator frequencies (Philicorda neon generator, Tunnel Diode tank) are designed to sweep down into sub-audio ranges ($0.01\text{Hz} - 50\text{Hz}$) to act as smooth cyclic modulators.
3.  **Discrete Trigger/Gate Output**: Sequencers and clock dividers output binary impulses (gate: high for step duration, trigger: pulse on step start) to trigger external behaviors.

---

## 2. Control Output Mapping Matrix

| Synthesis Module | Primary Control Output Type | Target Parameter / Behavior | Voltage / Value Scale |
| :--- | :--- | :--- | :--- |
| **🎹 Philicorda Neon Generator** | LFO / Sub-octave Divider | Cyclical parameter sweeps (e.g. searching depth modulation) | $\pm 1.0\text{V}$ (scaled by $10^{18}$) |
| **🌀 Tunnel Diode VCO** | High-Frequency NDR Oscillator | Rapid jitter, exploration noise, chaotic search | $\pm 0.5\text{V}$ (scaled by $10^{18}$) |
| **💨 Steam Whistle (Flame Mode)** | Chaotic Low-Frequency Noise | Random exploration rate, temperature scheduling in LLM search | $0.0\text{V} - 1.0\text{V}$ |
| **🗣️ Formant Filter** | Resonant Envelope ($I^2 + Q^2$) | Dynamic rate-limiting thresholds (Auto-attenuation sensor) | $0.0\text{V} - 2.5\text{V}$ |
| **🎵 SGS Rhythm Sequencer** | Bit-packed Trigger Gate Pulse | Step-wise task execution, database commit synchronization | Bitmask (0 to 7) |
| **⚡ Bionika Neuron** | Leaky Integrator Spike Train | Event-driven neural exploration, scheduling intervals | $0.0\text{V}$ or $1.0\text{V}$ |

---

## 3. Engineering Guidelines for CV Generation in Yul

To write stable control-generation code on the ZMM VM:

### A. Avoid Accumulator Overflows at Low Frequencies
When integrating over time at very slow frequencies ($dt \approx \text{large}$), step increments must be calculated with high precision to prevent rounding to zero or numeric overflows. Use signed fixed-point division scaling ($10^{18}$) carefully:
```yul
// Correct low-frequency integrator update
let delta := sdiv(mul(rate, dt), SCALE())
accumulator := add(accumulator, delta)
```

### B. Implement Slew-Rate Limiting (Control Smoothing)
To prevent sudden digital steps in control values from causing transient errors in downstream targets, implement a configurable slew-rate limiter inside control generators:
```yul
let diff := sub(targetValue, currentVal)
let maxStep := sdiv(slewRate, sampleRate)
if sgt(diff, maxStep) { diff := maxStep }
if slt(diff, sub(0, maxStep)) { diff := sub(0, maxStep) }
currentVal := add(currentVal, diff)
```

### C. Output Normalization
Control outputs must always scale to standard mathematical representations (such as $[-1.0, 1.0]$ or $[0.0, 1.0]$ in $10^{18}$ fixed-point representation) before transmission, allowing easy rescaling by downstream targets.
