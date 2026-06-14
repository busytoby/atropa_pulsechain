# Timing Accuracy and Clock Synchronization Standards

This document establishes the precise timing and synchronization parameters required to maintain cycle accuracy, prevent execution drift, and eliminate clock jitter between the virtual machine guest contracts and the host-side simulator interface.

---

## 1. Core Principles of Timing Accuracy

To ensure true deterministic behavior (especially for raster splits, sound synthesis, and real-time interactive debugging), the platform relies on three synchronization layers:

```
┌────────────────────────────────────────────────────────┐
│             Cycle-Accurate Instruction Loop            │
│  (Tracks exact cycles spent per opcode; e.g. NOP = 2)  │
└───────────────────────────┬────────────────────────────┘
                            ▼
┌────────────────────────────────────────────────────────┐
│             Interrupt Vector Synchronization           │
│   (Synchronizes raster line sweeps with vertical sync) │
└───────────────────────────┬────────────────────────────┘
                            ▼
┌────────────────────────────────────────────────────────┐
│             Host-Side Web Audio / Video Buffering     │
│   (Locks audio node scheduling to browser frame clock) │
└────────────────────────────────────────────────────────┘
```

---

## 2. Cycle-Accurate Instruction Budgets

In our virtual processors (such as the 6502 and 8080 emulators), execution speed is not governed by real-world CPU speed but by strict **cycle tracking**:
- **Clock Accumulation**: Every decoded opcode increments a global `cycles_executed` counter by the exact number of cycles the physical hardware would require (e.g., immediate loading `LDA` takes 2 cycles, absolute indexing `LDA addr, X` takes 4-5 cycles).
- **On-Chain Gas Alignment**: The on-chain execution step budgets are capped dynamically. When running step operations via `runDebugger`, the virtual debugger counts execution cycles to ensure state changes occur exactly when expected, preventing drift.

---

## 3. High-Precision Raster Timing (VIC-II)

Raster interrupt accuracy requires matching the horizontal sweep frequency of vintage video displays:
- **PAL Standard**: 50 Hz frame rate, 312 scanlines per frame, 63 cycles per scanline.
- **NTSC Standard**: 60 Hz frame rate, 263 scanlines per frame, 65 cycles per scanline.
- **Interrupt Alignment**: The guest code registers the target interrupt line in memory offset `$D012` (`53266`). The emulator triggers the interrupt handler vector (`$0314–$0315`) exactly when the instruction cycle count aligns with the target scanline sweep.

---

## 4. Jitter-Free Audio Synchronization (SID ADSR)

To eliminate pops, clicks, or phase cancellations during audio playback:
1. **Look-Ahead Scheduling**: Web Audio audio-parameter changes (frequency, waveform, envelope status) are scheduled `15ms` ahead of execution to compensate for thread latency.
2. **Phase Resetting**: Whenever the oscillator gate bit drops (triggering the Release phase of the ADSR envelope), the virtual voice phase is zeroed out to prevent remaining DC offset spikes from polluting the output buffer.
