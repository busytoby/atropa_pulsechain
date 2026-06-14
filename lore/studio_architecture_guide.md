# TSFi2 Synthesis & Telemetry Studio: Architecture & Module Guide

This document defines all the components comprising the **TSFi2 Synthesis & Telemetry Studio**. The studio combines physical analog circuit modeling in Yul bytecode (executed on the ZMM VM) with real-time visual telemetry and interactive controls in the web interface.

---

## 1. Physical Synthesis Modules (The Core Sound Engines)

These modules run statefully on the ZMM VM to generate raw audio signals:

### A. 🎹 Philicorda Neon Generator
*   **Source File**: [`philicordaGenerator.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/philicordaGenerator.yul)
*   **Description**: Models the Philips Philicorda electronic organ. It simulates neon gas-discharge relaxation oscillation (producing a raw sawtooth wave) coupled with a bistable frequency divider (generating a sub-octave square wave).

### B. 🌀 Tunnel Diode VCO
*   **Source File**: [`tunnelDiodeOscillator.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/tunnelDiodeOscillator.yul)
*   **Description**: Models a high-frequency oscillator using the Negative Differential Resistance (NDR) region of a Germanium tunnel diode coupled to a parallel LC resonant tank, producing warm, harmonically rich waveforms.

### C. 🧪 Germanium Common-Emitter Stage (Issue 1)
*   **Source File**: [`germaniumStage.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/germaniumStage.yul)
*   **Description**: Models an OC71 Germanium transistor gain stage. Solves base-emitter junction current non-linearity ($V_{be}$-$I_b$) using a signed Newton-Raphson solver. Features dynamic capacitor discharge paths to simulate bias sag and recovery under heavy signal drive.

### D. 💨 Steam Whistle & Flickering Flame (Issue 1)
*   **Source File**: [`steamWhistle.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/steamWhistle.yul)
*   **Description**: A dual-mode sound generator.
    *   **Whistle Mode**: TUP-TUN relaxation oscillator producing a whistling tone mixed with filtered avalanche breakdown noise.
    *   **Flame Mode**: Slows the oscillator to low frequencies ($2\text{Hz} - 15\text{Hz}$) modulated by noise to generate roaring, crackling fire crackles.

### E. 🥁 Membrane Drum Synthesizer (Issue 2)
*   **Source File**: [`drumSynthesizer.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/drumSynthesizer.yul)
*   **Description**: A state-variable tonal drum generator that models a spring-mass-damper membrane kicked by trigger velocity impulses to create damped tom-tom/kick hits.

### F. 🥁 Snare & Metal Drum Synthesizer (Issue 2)
*   **Source File**: [`snareSynthesizer.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/snareSynthesizer.yul)
*   **Description**: Mixes a tonal membrane resonator with an internal pseudo-random noise generator shaped by a fast-decaying envelope to synthesize snare drums, hi-hats, and metallic hits.

---

## 2. Processing & Filtering Modules (The Shapers)

These modules shape, filter, or limit the generated synthesis signals:

### A. 🌀 Gyrator Active Filter (Issue 2)
*   **Source File**: [`gyratorFilter.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/gyratorFilter.yul)
*   **Description**: Simulates active inductor-capacitor (LC) bandpass filters using active simulated gyrators ($L_{eq} = R_1 R_2 C$) in a Chamberlin State-Variable Filter (SVF) configuration.

### B. 🎚️ Equa-Amplifier Master AGC Limiter (Issue 1)
*   **Source File**: [`equaAmplifier.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/equaAmplifier.yul)
*   **Description**: Master leveler enforcing dynamic Safe Operating Area (SOAR) current limits ($I_{limit} = 1.97\,\text{A}$). Adjusts the output voltage window based on load impedance ($1.0\text{V}$ for $0.51\,\Omega$; $1.5\text{V}$ for $0.76\,\Omega$) to prevent digital clipping.

---

## 3. Control & Telemetry Interfaces (The Dashboard)

The visual and interactive layers connecting the user to the ZMM VM environment:

### A. 📺 Live Oscilloscope Canvas
*   **Description**: Renders real-time output signal voltages dynamically on a canvas plot. Custom trace colors distinguish active modules (e.g., green for Drum, purple for Gyrator, blue for Whistle).

### B. ⚡ Nixie Telemetry Indicator (Issue 4)
*   **Source File**: [`z550mIndicator.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/z550mIndicator.yul)
*   **Description**: Displays program counters or active signal amplitudes on a virtual decade grid, emulating the Philips Z550M neon decade counting tube.

### C. 🧪 Debugger Link (Ubiquitous AI Debugging)
*   **Source File**: [`debugger.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/debugger.yul)
*   **Description**: Exposes register-level execution tracing and step-by-step assembly diagnostics of active VM state variables.

### D. 🎹 Interactive Controller Keyboard
*   **Description**: 13-note musical keyboard allowing users to play notes and dynamically set oscillator pitch coefficients in real-time.
