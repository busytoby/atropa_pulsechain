# PL/EXUS: The Unified ALU of XplOS

Under the **Auncient** Dysnomia virtual machine design, PL/EXUS is not merely a language or execution environment; it is the **Unified ALU** of XplOS. It represents the mathematical and electrical convergence of three core operational layers:

```
                          ┌──────────────────────────┐
                          │     PL/EXUS Unified      │
                          │   ALU Execution Cycle    │
                          └────────────┬─────────────┘
          ┌────────────────────────────┼────────────────────────────┐
          ▼                            ▼                            ▼
  ┌───────────────┐            ┌───────────────┐            ┌───────────────┐
  │ WinchesterMQ  │            │ Network DAT   │            │ ABI Accessors │
  │ SCSI Phase    │            │ L2 Processing │            │ Capability    │
  │ Modulations   │            │ (STANAG/DEC)  │            │ Gated Bounds  │
  └───────────────┘            └───────────────┘            └───────────────┘
```

### 1. WinchesterMQ (SCSI Phase Modulations)
SCSI handshake loops do not run as isolated peripheral drivers. The prime field modulations ($Base^{Signal} \pmod{MotzkinPrime}$) are resolved as core ALU instructions, translating calculation results directly into coordinate phase displacements ($\phi_w$) to shape the projected wireframe envelope.

### 2. Network DAT (L2 Processing)
STANAG and DECnet device state tables are wired directly into the 2-3 tree Merkle DAT. Register updates on these devices are handled as direct ALU store operations. This guarantees that frame transactions dynamically update the parent hashes, producing a cryptographically audited state ledger at the hardware boundary.

### 3. ABI Accessors (Capability Gating)
The Application Binary Interface (ABI) enforces memory bounds and context privilege tags at the gate level. During execution, the ALU evaluates privilege tags using series/parallel logic networks (`bjt_and`, `bjt_not`). Unauthorized memory accesses or out-of-bounds calculations trigger a `TRANSISTOR_CUTOFF` on the write bus, isolating registers instantly.

By unifying virtual hardware loops, network interfaces, and privilege checks into a single BJT switching execution cycle, PL/EXUS eliminates operating system software overhead, guaranteeing absolute security and constant-time execution.
