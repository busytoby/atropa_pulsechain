# MTAC Issue #12: W. J. Eckert, Punched Cards, and Wavetable Synthesis

This document provides a detailed technical analysis of Wallace J. Eckert’s paper *"Mathematical Tables on Punched Cards"* (MTAC Volume 1, Number 12, October 1945) and its direct application to wavetable synthesis and dynamic address generation in digital synthesizers.

---

## 1. Eckert's Punched Card Tables

During World War II, W. J. Eckert coordinated the calculation of astronomical and mathematical tables using IBM tabulating machines:
* **The Concept**: Instead of computing values (like logarithms, sines, or Bessel values) on the fly, they pre-calculated massive catalogs of values onto decks of punched cards.
* **The Method**: Card sorting machines and tabulators were configured to perform fast, automated lookups. By sorting cards based on the input argument, the machine could retrieve the corresponding pre-calculated function value instantly, shifting the computational burden from active calculation to sorting and retrieval.

---

## 2. DSP Application: Digital Wavetable Synthesis

In digital synthesizers, the equivalent of Eckert's punched card deck is **Wavetable Synthesis**:

```
                       Input Phase (Argument)
                                 │
                                 ▼
                     +-----------------------+
                     |  Eckert Wavetable LUT |
                     |   (Pre-calculated)    |
                     +-----------┬-----------+
                                 │
                                 ▼
                       Output Sample Value
```

### A. Pre-calculation and Address Mapping
Instead of executing complex trigonometric series expansions or differential equations every sample tick (which consumes ZMM VM instruction cycles), we store one cycle of the target waveform (e.g. a sine, square, or custom Bessel function) in a **Look-Up Table (LUT)**:
1.  **Static Storage**: The wave is sampled at $N$ points (e.g., $512$ samples) and stored in contract memory.
2.  **Dynamic Phase Accumulation**: As the simulation ticks, we increment a phase pointer $P$:
    $$P = (P + \Delta P) \pmod N$$
3.  **Instant Lookup**: The pointer $P$ acts as the index to retrieve the pre-calculated sample. 
4.  **Eckert-style Retrieval**: By using integer phase indexes, this method matches the card retrieval speed of Eckert's punched card decks, allowing the ZMM VM to generate high-fidelity, complex waveforms with zero division or floating-point instruction cost.
