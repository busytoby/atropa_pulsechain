# Kilobaud Issue #1: Zero-Crossing Opto-Triacs, Solid-State Power Shunting, and Resistor Video Mixing

This document provides a detailed technical analysis of the engineering and hardware principles presented in *Kilobaud Computing* Issue #1 (January 1977), focusing on how zero-crossing solid-state relays (SSRs) and resistor video mixing improve the performance and visual rendering of the **TSFi2 Synthesis Studio**.

---

## 1. Zero-Crossing Solid-State Relays (Computer Control of the World)

In his article *"Computer Control of the World"*, Chris Bowick details interfacing microcomputers to high-voltage AC loads using **opto-triacs** (zero-crossing optical isolators) and solid-state relays (SSRs). 

### A. The Principle of Zero-Crossing Switching
Switching an inductive or capacitive load when the AC mains voltage is at its peak ($V_{peak}$) creates massive electromagnetic interference (EMI), voltage spikes, and contact arcing. 
* By incorporating a zero-crossing detector, the opto-triac delays firing the gate until the AC voltage crosses exactly through $0\text{V}$.
* This minimizes switching surge currents and prevents click transients.

### B. Application: Click-Free Zero-Crossing Power Shunt
In our synthesis studio, dynamic gain throttling or shunt protection can introduce immediate steps (clicks) into the audio stream if switched on/off instantly mid-waveform.

By modeling a virtual **Zero-Crossing SSR**:
- Protective thermal throttling and load shunting transitions are delayed and synchronized to occur *only* when the audio input waveform crosses zero.
- This guarantees completely click-free protection switching, preventing transient pops during active thermal limiting.

---

## 2. TV Tournament Resistor Video Mixing (TV Tournament Time)

The issue reviews dedicated video game chips (such as the General Instrument AY-3-8500 Pong chip). These chips output sync, background, ball, and player channels as individual digital lines.
* **Resistor Mixer**: These channels are mixed using a simple passive resistor network to create a composite video signal for the TV.
* **Application**: We model this resistor-matrix mixing on our dashboard canvas, overlaying the oscilloscope signal, Nixie glow, and speaker sprite movement with proper composite luminance weighting.
