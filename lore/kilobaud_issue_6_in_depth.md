# Kilobaud Issue #6: TTL Pulse Generators, SC/MP Architecture, and Duty Cycle Control

This document provides a detailed technical analysis of the hardware concepts presented in *Kilobaud Computing* Issue #6 (June 1977), focusing on adjustable pulse generators and the SC/MP microprocessor.

---

## 1. TTL Pulse Generators (Build a Pulse Generator)

In their article *"Build a Pulse Generator"*, Bob Grater and George Young outline the design of a TTL-based lab pulse generator.

### A. Core Parameters
Unlike simple fixed-frequency square wave oscillators, a professional pulse generator permits control over:

1. **Pulse Repetition Frequency (PRF)**: The overall rate of pulses per second (Hz).
2. **Duty Cycle / Pulse Width**: The ratio of the active high duration to the total period:
   $$\text{Duty Cycle} = \frac{T_{\text{on}}}{T_{\text{on}} + T_{\text{off}}} \times 100\%$$
   Adjusting the duty cycle alters the harmonic spectrum of the output signal (e.g. 50% duty cycle generates only odd harmonics, whereas narrow pulses produce all harmonics at equal strength).

### B. Application: Virtual TTL Pulse Generator
We implement a virtual **TTL Pulse Generator** as a primary clock source in our synthesis studio:
- Provides adjustable rate and duty cycle parameters.
- Clock signals are routed to trigger the **Rhythm Sequencer** and charge-discharge states in the **Bionika Spiking Neuron**, demonstrating how duty cycle offsets govern neural accumulation speeds.

---

## 2. The SC/MP Microprocessor (The World's Cheapest Computer)

Richard Hogg reviews the National Semiconductor SC/MP (Simple Cost-effective Micro Processor, or "Scamp").
* **Low-Cost Control**: SC/MP was designed as a controller rather than a general-purpose CPU.
* **Serial I/O Pins**: It featured native, on-chip serial input (`SIN`) and serial output (`SOUT`) pins, allowing the processor to communicate without external UARTs or interface adapters.
* **Pointer Registers**: Contained three 16-bit pointer registers ($P1, P2, P3$) that supported auto-indexing, making table lookups and vector sweeps extremely efficient in simple assembly loops.
