# Auncient VM Accelerated Computing Specifications

This document outlines the hardware acceleration options designed for the Auncient VM. These options are documented for architectural completeness and layout mapping but are not active in current software implementations.

---

## 1. Junction-Level Gating (Zero-Instruction Contracts)
Instead of compiling PDL/APDL assertions ($\phi \implies \psi$) into CPU software instruction branches, the accelerated architecture maps these predicates directly to physical hardware junctions:
* **NPN Black Gate**: Precondition $\phi$ is mapped to the active-high base pin of the NPN transistor.
* **PNP Red Gate**: Postcondition $\psi$ is mapped to the active-low base pin of the PNP transistor.
* **Mechanism**: Logical evaluation is performed via current conduction. If the contract fails, the write path cuts off in 0 clock cycles, preventing bus writes without software execution overhead.

---

## 2. Hardware-Shadowed Converse Recovery (Single-Clock Rollback)
To accelerate state recovery during Converse ($\alpha^-$) rollbacks, the guest register files are paired with dual-port shadow SRAM:
* **Mirroring**: Every write operation to a primary VDM register automatically mirrors the prior value to the adjacent shadow cell.
* **Trigger**: Upon an exception trap, a hardware cascade line flashes the shadow cells back to the active registers in parallel, executing the rollback in a single clock cycle.

---

## 3. Resonant Analog Demultiplexers (Fourier Gates)
AUTODIN coaxial packet frame inspection is accelerated at the physical network interface:
* **Mechanism**: Incoming signal lines are routed through analog bandpass filter circuits tuned to the $440\text{Hz}$ carrier frequency.
* **Result**: Off-frequency signals and noise are filtered out at the physical layer, preventing unaligned frames from triggering interrupts or consuming CPU buffer memory.
