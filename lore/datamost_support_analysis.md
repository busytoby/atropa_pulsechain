# DATAMOST Platform Emulator Support Analysis

This document provides a comprehensive evaluation of our decentralized, EVM-backed Commodore 64 simulation console (DATAMOST), analyzing the smart contracts, memory layout, graphics pipeline, sound synthesizer, and disk interfaces implemented so far.

---

## 1. Unified Architecture & Registry

The DATAMOST system leverages a decentralized microkernel model where independent, optimized Yul contracts serve as virtual hardware components.

```mermaid
graph TD
    subgraph Local Anvil Node (EVM Engine)
        CPU[cpu6502.yul]
        VIC[graphicsSystem.yul]
        SID[musicMaker.yul]
        DFS[diskSystem.yul]
    end

    subgraph Client Applications
        DApp[datamost.html / app.js]
        Vulkan[tsfi_vulkan Native Host]
    end

    DApp -->|runSteps / poke| CPU
    DApp -->|getSIDState| SID
    DApp -->|checkCollisions / updateSprite| VIC
    DApp -->|executeDiskCommand| DFS
```

The system contracts are deployed deterministically at these active local addresses:
* **Disk System (DFS):** `0xcc83F5064a4eF67d6e3Ee5B5D93eB04b75e0842c`
* **Music Maker (SID):** `0x3B1B55CBb9191D61087242Def748C800EE86d78c`
* **CPU 6502 Interpreter:** `0x41Afe93e85609a1afD8E272A75C11Abd1265E0c8`

---

## 2. Memory-Mapped Register System

The contracts emulate classic Commodore 64 hardware state by storing register configurations in specific EVM storage slots:

### A. CPU 6502 Register Mapping (`cpu6502.yul`)
* **Slot `0x80`:** Accumulator A
* **Slot `0x81`:** X Register
* **Slot `0x82`:** Y Register
* **Slot `0x83`:** Stack Pointer (SP) — emulates Stack RAM range `0x100` to `0x1FF`.
* **Slot `0x84`:** Status Register (SR) — flags Carry (bit 0), Zero (bit 1), Overflow (bit 6), and Negative (bit 7).
* **Slot `0x85`:** Program Counter (PC)

### B. VIC-II Sprite & Color Mapping (`graphicsSystem.yul`)
* **Slots `53248` - `53263` (`$D000` - `$D00F`):** Sprite 0–7 X/Y coordinates.
* **Slot `53269` (`$D015`):** Sprite Enable Register.
* **Slot `53278` (`$D01E`):** Sprite-to-Sprite Collision Register.
* **Slot `53280` (`$D020`):** Border Color Register.
* **Slot `53281` (`$D021`):** Background Color Register.
* **Slots `54000`+:** Sprite custom bitmap staging cells (each sprite maps a 24x21 canvas).

### C. SID Sound Registers (`musicMaker.yul`)
* **Slots `54272` - `54296` (`$D400` - `$D418`):** Full 25-byte register block representing voice parameters:
  * Voice 1: Frequency (`$D400`-`$D401`), Waveform (`$D404`), ADSR (`$D405`-`$D406`).
  * Voice 2: Frequency (`$D407`-`$D408`), Waveform (`$D40B`), ADSR (`$D40C`-`$D40D`).
  * Voice 3: Frequency (`$D40E`-`$D40F`), Waveform (`$D412`), ADSR (`$D413`-`$D414`).

---

## 3. High-Fidelity Synthesizer & Video Pipelines

### A. 3-Voice Web Audio SID Emulator
We implemented a robust sound synthesizer on the frontend that replicates retro sound generators:
* Handles **Sawtooth, Triangle, Square, and Noise** oscillators mapped directly from on-chain control registers.
* Simulates physical ADSR envelope curves (converting discrete register hex indexes to linear/exponential ramps).

### B. VIC-II Bounding Collisions & Sprite Rendering
* **On-Chain Collisions:** The graphics contract performs a bounding check on active 24x21 blocks to set collision bits inside `$D01E`. The CPU reads this collision register, executes branch redirections, and increments the score register `$0003`.
* **Dynamic Palette:** Clears the border canvas using colors parsed from the border register (`$D020`) and draws the playfield based on the background register (`$D021`) matching the authentic 16-color Commodore palette.

---

## 4. Floppy Disk File Manager (DFS)

The Virtual 1541 Disk System contract (`diskSystem.yul`) tracks directory slots starting at slot `0x1000`. Filenames are indexed with dynamic operations:
* **`R0:$` (Directory listing):** Returns active filenames.
* **`W0:<name>` (Write file):** Writes up to 256 bytes of raw program payload.
* **`R0:<name>` (Read file):** Recalls raw hex binaries from floppy storage slots.
* **`S0:<name>` (Scratch file):** Cleans filename mappings and clears storage cells.

---

## 5. Completed vs. In-Progress Milestones

| System Module | Feature Component | Status |
| :--- | :--- | :--- |
| **CPU 6502 Emulator** | Zero-Page loads/stores (`$A5`/`$85`) | **Completed** |
| | Accumulator comparisons (`$C9`/`$CD`/`$C5`) | **Completed** |
| | Register X/Y comparisons (`$E0`/`$C0`) | **Completed** |
| | Absolute increments/decrements (`$EE`/`$CE`) | **Completed** |
| **SID Synthesizer** | 3-Voice synthesis & ADSR curves | **Completed** |
| **VIC-II Video** | Custom Sprite Bitmaps (24x21) | **Completed** |
| | Dynamic Palette Registers ($D020/$D021) | **Completed** |
| | Screen RAM character maps (1024-2023) | **Completed** |
| **Disk DOS (DFS)** | Sector Read/Write/Scratch & Directories | **Completed** |
