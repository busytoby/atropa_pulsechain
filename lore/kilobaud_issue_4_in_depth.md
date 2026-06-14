# Kilobaud Issue #4: Eight-Channel Scope Multiplexers, Hardware Interrupts, and Early Digital Audio

This document provides a detailed technical analysis of the hardware concepts presented in *Kilobaud Computing* Issue #4 (April 1977), focusing on how multi-channel oscilloscope multiplexing and digital audio architectures apply to the **TSFi2 Synthesis Studio**.

---

## 1. Eight-Channel Scope Multiplexing (Build an Eight Channel Multiplexer)

In the article *"Build an Eight Channel Multiplexer for Your Scope"*, W.J. Prudhomme details a hardware design that allows a standard single-channel oscilloscope to display up to eight independent waveforms simultaneously.

### A. The Multiplexing Mechanism
Since a single electron gun in a CRT can only trace one path, the multiplexer high-speed switches between 8 input channels at a rate faster than the persistence of human vision.
1. **Vertical Offset Stepping**: To prevent the traces from overlapping and scrambling into a single line, the multiplexer adds a distinct, calibrated DC bias offset to each channel. This stacks the traces vertically on the screen.
2. **High-Speed Clocking**: Uses a digital counter to step through the channels continuously.

### B. Application: Multi-Trace Virtual Scope
In our dashboard canvas scope, instead of rendering only the active module's output trace, we can implement a **Virtual Multiplexer**:
- Displays the **Active Waveform** (Channel 0), **Bionika Nerve Spikes** (Channel 1), and **CCS Attenuation Control Voltage** (Channel 2) simultaneously.
- Stacks them vertically by adding distinct vertical offsets, giving the user a complete, real-time diagnostic view of the entire system state on a single screen.

---

## 2. Early Digital Audio (Digital Audio)

Tom Scott's *"Digital Audio"* article explores representing audio waveforms as binary data blocks using early D/A converters (like the Motorola MC1408 8-bit DAC).
* **Sampling Rate Limits**: Notes the strict trade-off between processor clock speeds and audio bandwidth.
* **Resolution Limits**: Discusses how 8-bit quantization introduces audible quantization distortion, which we mitigate in our studio using Zener-avalanche dynamic dither injection to disperse limit cycles.
