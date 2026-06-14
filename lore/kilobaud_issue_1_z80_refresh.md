# Kilobaud Issue #1: Z80 Hardware DRAM Refresh and State-Leakage Mitigation

This document details the architecture of the Zilog Z80's built-in Dynamic RAM (DRAM) refresh controller, as discussed in *Kilobaud* Issue #1 (January 1977), and explores how this concept applies to modeling capacitor charge leakage and state refresh cycles in the **TSFi2 Synthesis Studio**.

---

## 1. The Z80 Hardware DRAM Refresh Mechanism

In 1977, Dynamic RAM (DRAM) was much cheaper than Static RAM (SRAM) but required periodic electrical refreshing (recharging the storage capacitors in each memory cell every few milliseconds) to prevent data loss.
* **The Z80 Solution**: Zilog integrated an automatic refresh controller directly into the CPU silicon.
* **The R (Refresh) Register**: The Z80 includes a 7-bit hardware register (`R`) that increments automatically during the instruction fetch cycle (M1).
* **Transparent Refresh**: While the CPU is internally decoding the fetched instruction, the Z80 puts the value of the `R` register onto the lower half of the address bus along with a dedicated `RFSH` signal, refreshing a row of DRAM transparently with zero execution overhead.

---

## 2. Application: State-Leakage & Refresh in Physical Synthesis

In physical modeling (such as our Bionika neuron and Philicorda RC circuits), capacitors lose charge over time due to leakage currents. In a digital VM, if states are stored without active degradation limits, they remain perfectly static, lacking the organic drift of analog hardware.

To model analog hardware accurately:
- **Capacitive State Decay**: State variables (like filter capacitors or memory register values) leak charge at a rate governed by:
  $$V(t + \Delta t) = V(t) \cdot e^{-\lambda \Delta t}$$
- **State Refresh Cycles**: A virtual "Refresh Instruction" (resembling the Z80 `RFSH` cycle) periodically re-stabilizes bias references and restores capacitor charges, preventing decay from collapsing the sound engines.
- **Acoustic Result**: Generates organic drift, warm instability, and low-frequency "breathing" in filter sweeps as the refresh intervals modulate the decay rates.
