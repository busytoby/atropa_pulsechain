# Auncient XplOS Debugging & Profiling Architecture

This document describes the native, hardware-gated debugging and execution profiling infrastructure designed for XplOS binaries within the Dysnomia VM framework.

## 1. Architectural Design

The profiling and debugging pipeline consists of three core components that coordinate directly with the physical BJT transistor simulation and the cryptographically audited 2-3 tree DAT layout:

```
        Dysnomia VM Execution Pipeline (PC, ACC)
                         │
                         ▼
           [ Hardware Debug Inspector ]
                         │
    ┌────────────────────┼────────────────────┐
    ▼                    ▼                    ▼
[ Breakpoint Gate ]  [ Limit Auditor ]   [ Trace Logger ]
    │                    │                    │
    ├─ PC == Break?      ├─ ACC > Max ACC?    └─ Store Trace Leaf
    ▼                    ▼                       in 2-3 Tree DAT
TRANSISTOR_CUTOFF    TRANSISTOR_CUTOFF
```

### Hardware-Gated Debug Intercepts (Breakpoints)
Unlike standard operating systems that rely on software interrupt vector traps (e.g. `INT 3` or `bkpt`), XplOS utilizes hardware-gated assertions. The debugger registers a target breakpoint address. When the Program Counter register ($PC$) matches this breakpoint, it drops the base current of the gating NPN/PNP transistor structures to zero. 
* **State Isolation**: This halts register writes instantly via a physical `CUTOFF_STATE`, freezing the execution context without modifying memory space or registers.

### Active Threshold Limit Audits
The debugger monitors register boundaries (such as the accumulator $ACC$) dynamically. If a mathematical execution cycle would force a register beyond safe physical boundaries (such as a simulated voltage overflow or data overcurrent), a lookahead gate preemptively asserts a `CUTOFF_STATE`, protecting the register space and triggering a hardware breakpoint event.

### Merkle-Audited Trace Registry
Every successfully executed instruction step is profiled, extracting telemetry parameters ($PC$, $ACC$, $Tag$). These traces are saved as leaves in the 2-3 tree DAT table layout. Because each entry updates the parent node hashes, a cryptographically signed execution log is generated.

## 2. Telemetry and Glossary Terms

### DebuggerState
* **VM Register Context**: The low-level state register in the Dysnomia VM that tracks the transition between step execution and debug halt cycles. It is updated via the hardware breakpoint register comparison circuit.
* **Mathematical Function**: Resolves the state transition index:
  $$S_{\text{debug}} = 1 - \prod_{i} (PC_i \oplus Breakpoint_i)$$
* **Visual / Geometric Shift**: Modulates the rendering opacity and coordinates of the execution status bars in the debugger visualizer dashboard, turning lines to a high-contrast amber indicator when halted.

### TraceHash
* **VM Register Context**: The registry pointer matching the current step's execution trace leaf in the 2-3 tree DAT.
* **Mathematical Function**: Computed recursively at each step execution:
  $$H_{\text{trace}} = \text{SHA256}(PC \,||\, ACC \,||\, Tag \,||\, LastHash)$$
* **Visual / Geometric Shift**: Controls the orbital paths and translation displacements of the coordinate envelopes in the tesseract visualizer, generating distinct rotation speeds linked to the cryptographic uniqueness of the execution trace.
