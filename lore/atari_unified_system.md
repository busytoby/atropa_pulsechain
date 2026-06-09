# Atari Unified Coprocessor Architecture

This document describes the unified integration of Atari's flagship graphics and sound coprocessor architectures: **ANTIC**, **GTIA**, and **POKEY**. By offloading scrolling, display mode generation, interrupts, audio generation, and collision detection to dedicated hardware, the system maximizes the efficiency of the 6502 CPU.

---

## 1. Unified Coprocessor Pipeline

During each video frame, the subsystems operate in parallel:

```mermaid
graph TD
    CPU[6502 CPU] -->|1. Setup Display List & Registers| Memory[System RAM]
    Memory -->|2. DMA Fetch Instructions & Pixels| ANTIC[ANTIC Coprocessor]
    ANTIC -->|3. Fine Scrolling HSCROL/VSCROL| ANTIC
    ANTIC -->|4. Trigger Mid-Screen interrupts| DLI[Display List Interrupts]
    ANTIC -->|5. Pixel Index Stream| GTIA[GTIA Color & Sprites]
    GTIA -->|6. Collision Check PxPL/PxPF| GTIA
    GTIA -->|7. Analog Inputs / Buttons| POT[POT Registers]
    POKEY[POKEY Sound Chip] -->|8. Audio Dividers & Poly-counters| Speaker[Audio Output]
```

### Subsystem Interoperability

1. **ANTIC & GTIA Co-operation**: ANTIC resolves physical layout mapping (such as Display List instructions and scroll offsets) and sends high-level mode structures to GTIA. GTIA applies color translation palettes and overlays hardware player-missile sprites.
2. **GTIA Collisions**: As sprites and background pixels overlap on the presentation beam, GTIA updates hardware registers instantly, making collision checking a zero-CPU cost operation.
3. **POKEY Audio**: Operates independently, division loops and poly-counters (LFSRs) run in parallel, triggered by CPU register modifications.

---

## 2. Integrated Simulation

We have created a unified demonstration harness: [atari_unified_system.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/atari_unified_system.js).

This script simulates a complete virtual frame loop incorporating:
* **ANTIC scrolling & display list resolution**
* **Display List Interrupt (DLI) palette shifts**
* **GTIA pixel overlap collision mapping**
* **POKEY audio frequency calculations**
* **Collision register resets (`HITCLR`)**
