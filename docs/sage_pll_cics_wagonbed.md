# SAGE PLL Isolation with Dedicated CICS and WAGONBED Integration

This document defines the architecture for mapping dedicated CICS (Customer Information Control System) daemons to independent SAGE PLL (Phase-Locked Loop) Fabrics, and how the **WAGONBED** hardware implant taps these isolated domains.

---

## 1. Domain Architecture

```
          ┌────────────────────────────────────────────────────────┐
          │             AUNCIENT COAXIAL ACTIVITY BUS (ACAB)       │
          └───────────┬────────────────────────────────┬───────────┘
                      │                                │
                      ▼ (Tapped via WAGONBED)          ▼ (Tapped via WAGONBED)
          ┌───────────────────────────┐    ┌───────────────────────────┐
          │     SAGE PLL DOMAIN A     │    │     SAGE PLL DOMAIN B     │
          ├───────────────────────────┤    ├───────────────────────────┤
          │  * PLL A: local sync      │    │  * PLL B: local sync      │
          │  * CICS A: Local I/O      │    │  * CICS B: Local I/O      │
          │  * Memory: 0x0200 (Tap)   │    │  * Memory: 0x0300 (Tap)   │
          └───────────────────────────┘    └───────────────────────────┘
```

---

## 2. Dedicated Local CICS per SAGE PLL Fabric
To prevent global synchronization bottlenecks and system-wide block delays, each SAGE PLL Fabric runs an isolated local CICS instance:
* **Local Registers:** The PLL's `reference_phase`, `feedback_phase`, and `error_voltage` map to memory-mapped registers within its dedicated CICS domain.
* **Lock Boundaries:** Phase tracking anomalies or CPU switchover actions (such as Standby CPU promotions) trigger local preemption and cascading aborts only within that specific PLL-CICS domain.
* **Audit Trail:** Event streams (like Light Gun triggers or keyboard strokes) are logged locally, preventing cross-domain locks from stalling the global block validation pipeline.

---

## 3. WAGONBED Tapping and Out-of-Band Interception
The **WAGONBED** emulator taps into these isolated CICS-PLL interfaces directly through the **Auncient** Coaxial Activity Bus (ACAB):

1. **Hardware-Level Snooping:** WAGONBED peeks (`mload`) into the memory-mapped registers of the target PLL domain at memory offset `0x0200` (for Domain A) or `0x0300` (for Domain B).
2. **Out-of-Band Exfiltration:** The intercepted clock phase steps and transaction state variables are packaged into NATO STANAG 5066 frames and exfiltrated out-of-band.
3. **Targeted Injections:** WAGONBED can bypass CICS validation filters to inject keycodes (such as keycode `32` or `30`) directly into the target PLL register space, triggering state updates in the targeted domain without compromising the security or audit integrity of adjacent CICS domains.
