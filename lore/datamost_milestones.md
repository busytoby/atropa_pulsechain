# DATAMOST Emulator Roadmap & Milestones

This document establishes the key milestones for advancing our decoupled EVM-backed Commodore 64 simulation console (DATAMOST), covering the on-chain Yul engines, the native Vulkan rendering system, and the web-based DApp client.

---

## Milestone 1: 6502 CPU Instruction Set Completeness [COMPLETED]
Presently, the `cpu6502` contract implements basic immediate loads and register transfers. We need a fully functioning interpreter loop.
* **[COMPLETED] Support Full Addressing Modes:** Implemented absolute, zero-page, indirect indexed, and immediate modes.
* **[COMPLETED] Control Flow & Branching:** Implemented jumps, relative branches (BNE, BEQ, BCC, BCS, BPL, BMI), stack pushes/pulls (PHA, PLA, PHP, PLP), and subroutine jumps/returns (JSR, RTS).
* **[COMPLETED] Arithmetic & Flags:** Full flags alignment for Zero, Carry, and Negative indicators.

---

## Milestone 2: VIC-II High-Fidelity Video Emulation [COMPLETED]
* **[COMPLETED] Custom Sprite Bitmaps (24x21):** Support reading raw 63-byte C64 sprite data blocks from on-chain storage registers and rendering them dynamically on DApp canvas.
* **[COMPLETED] Dynamic Palette Registers:** Read storage slots `$D020` (53280) and `$D021` (53281) to render standard 16-color C64 border and background palettes.
* **[COMPLETED] C64 Character/Bitmap Modes:** Mapping Screen RAM cells dynamically.

---

## Milestone 3: Advanced SID Sound Architecture [COMPLETED]
* **[COMPLETED] 3-Voice Architecture:** Support all three standard SID voices simultaneously.
* **[COMPLETED] ADSR Envelopes:** Implement full Attack, Decay, Sustain, and Release envelope registers (`54277-54278` etc.) to model vintage synthesizers.
* **[COMPLETED] Waveform Selectors:** Support pulse-width modulation (PWM), noise, triangle, and sawtooth modes driven by registers.

---

## Milestone 4: On-Chain Virtual Disk System (DFS) [COMPLETED]
* **[COMPLETED] PRG/SEQ File Support:** Set up a Yul-based file mapping table inside the `diskSystem` contract.
* **[COMPLETED] Standard Commands:** Enable directory listings (`$`) and dynamic loading/saving of 6502 assembly programs from/to EVM storage.

---

## Milestone 5: DeepSeek Consensus & Physics Filters [COMPLETED]
* **[COMPLETED] Physics Collision Latch:** Fully synchronize the Yul local thunk collision check results with on-chain consensus state.
* **[COMPLETED] Consensus-Driven Inputs:** Allow multiple players to vote on inputs or frame state changes.
