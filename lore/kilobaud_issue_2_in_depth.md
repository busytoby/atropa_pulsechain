# Kilobaud Issue #2: COSMAC 1802 Register Pointer Switching and 6800 Single-Stepping Debuggers

This document provides a detailed technical analysis of the hardware concepts presented in *Kilobaud Computing* Issue #2 (February 1977), focusing on how the RCA 1802's dynamic program counter switching and Motorola 6800 hardware single-steppers apply to the **TSFi2 Synthesis Studio** VM.

---

## 1. RCA COSMAC 1802 Dynamic Register Switching (The P Register)

The RCA 1802 microprocessor (COSMAC) uses a unique register file architecture:
* **Register File**: Sixteen 16-bit registers ($R0$ to $R15$).
* **Dynamic PC Assignment**: Unlike conventional processors with a hardwired Program Counter (PC), *any* of the sixteen registers can act as the PC.
* **The P Register**: A 4-bit register `P` points to one of the 16 registers, designating it as the active PC.
* **Instant Subroutines / Interrupts**: Changing the active PC requires only a single instruction to write to `P` (`SEP R`). This swaps execution contexts instantly with zero stack push/pop latency.

### Application: Emergency Limiter Context Swapping
In our ZMM Virtual Machine:
- We model an **RCA 1802-style dual-PC structure**.
- **Register $R0$** points to the normal audio sample synthesis loop.
- **Register $R1$** points to the safety limit / clamping interrupt routine (Bionika reflex / Q-damping).
- When the ADC detects a clipping incident, the VM immediately swaps the execution register pointer to $R1$. The VM processes the clipping control loop instantly, then restores the pointer to $R0$ on the next cycle, preventing transient click leaks without stack overhead.

---

## 2. Motorola 6800 Hardware Single-Stepping (A 6800 Single Stepper)

Mark Borgerson’s article details using a flip-flop circuit to halt the Motorola 6800 microprocessor after every clock cycle (using the `HALT` pin) to read out the address and data buses for debugging.

### Application: Virtual Single-Stepping Console
We expose a single-stepping debug control in our studio. The user can halt the automatic simulation loop and step through Yul EVM execution trace blocks sample-by-sample, visualizing:
- Active registers ($R0 - R15$).
- Target memory slots.
- Calculated dynamic attenuation levels.
- The active Program Counter index.
