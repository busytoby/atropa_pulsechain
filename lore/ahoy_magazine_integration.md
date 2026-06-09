# Ahoy! Magazine Issue 1 (January 1984) Integration into TSFi2

This document details the mapping, theoretical alignment, and practical integration of retro-computing concepts from the premiere issue of **Ahoy!** magazine into the **TSFi2** (Bijective Helmholtz Architecture/RDNA 4 simulation engine).

---

## 1. Concept Mapping & Architecture Matrix

| Ahoy! Issue 1 Concept | Commodore 64 Subsystem | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Interrupt Music Maker / Editor** | C64 Kernal Interrupts (IRQ), SID Chip (MOS 6581) audio synthesis. | **Kirchhoff-Helmholtz Acoustic Resonance** | Emulating SID-like register modulation via virtual interrupts that drive periodic wavefield resonance updates. |
| **Multi-Draw 64 Graphics Tablet** | Hi-Res Bitmap (320x200), Joystick/Keyboard raster scanning. | **Voxel Graphics & Font Engine** | Matrix Cores & Voxel SVDAG coordinates mapping. Retro joystick/keyboard-driven cell manipulation in the cockpit. |
| **A Peek at Memory on the C64** | PEEK/POKE, 64KB RAM layout, PLA control lines. | **Resident-State/ZMM Register Files** | Direct manipulation of Boltmann/Feynman registers and wired allocation verification. |
| **Kids' Assembler (Datamost)** | 6502 assembly, CPU Registers, Opcodes (LDA, STA, etc.) | **Yul Virtual CPU Emulator** | Executing classic 6502 opcodes on EVM-mapped storage slots. |
| **Programming Sequential Files** | Commodore DOS 1541 disk command channel (SEQ files). | **Merkle Leaf Persistence & Stream Keys** | Linear log streams and state verification using continuous sequential files. |

---

## 2. Deep-Dive: Interrupt-Driven Acoustic Resonance

In Commodore 64 programming, the **Interrupt Music Maker** utilizes vertical blank or timer-based interrupts (60Hz or custom rates) to write frequency, envelope, and waveform parameters to the MOS 6581 SID chip. 

In the TSFi2 Bijective Helmholtz Architecture, we map this to:
1. **Interrupt Vector Emulator**: A simulated interval timer driving the `tsfi_helmholtz` simulation iterations at simulated periodic intervals.
2. **Frequency Registers (SID Voice Emulation)**: Modulating the Helmholtz resonance frequency ($k$) through time-varying step functions corresponding to Commodore 3-voice SID synthesis.
3. **Sound Register Pokes**: Providing memory-mapped virtual POKEs that alter simulated acoustic wavefield coefficients in the virtual manifold.

---

## 3. Deep-Dive: Multi-Draw Voxel Mapping & Joystick Controls

The **Multi-Draw 64** graphics system coordinates mapping is converted from a 2D high-resolution screen coordinates system to 3D SVDAG coordinates or Doodle canvas pixels:
* **2D Canvas** $\rightarrow$ **3D Voxel plane Projection / Doodle Canvas**.
* **Pixel Drawing** $\rightarrow$ **Voxel presence matrix density / Bitmap frame-buffer writes**.

### Verification of Multi-Draw 64:
We implemented a dedicated verification test at [scripts/test_multidraw.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_multidraw.js) to execute the core joystick-polling and pixel-drawing routines.

1. **Bytecode Loader**: Flashes the 6502 machine code program that reads joystick data from register `$DC00` (56320), updates screen coordinates in zero-page memory, and writes to Doodle graphics coprocessor registers when the fire button is pressed.
2. **Joystick Simulation**: Injects active-low joystick input (`0xE7` representing right direction + fire button).
3. **Graphics Output**: Runs instruction steps and asserts that:
   - Zero-page X coordinate incremented to `161` (representing movement).
   - Bitmap canvas memory byte at address `12196` had bit 6 set to `1` (indicating successful pixel plotting).

---

## 4. Integration Verification

We have implemented a verification test at [tests/test_ahoy_issue1_resonance.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_ahoy_issue1_resonance.c) and deployed a custom music synth register set at [solidity/bin/musicMaker.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/musicMaker.yul).

By querying these Yul registers on our local EVM during note transitions, we have compiled the note sequence into actual audio waves:
*   **Generated Music Output:** [music.wav](file:///home/mariarahel/.gemini/antigravity-cli/brain/5289e240-c025-43c9-95f2-79673251a341/music.wav) (A synthesized C4-E4-G4-C5 melody generated directly from on-chain state data).

---

## 5. Singular Factory Model Deployments (CREATE2)

To ensure deterministic addresses and enforce a singular deployment model across persistent and local networks, all Yul contracts are deployed via the pre-deployed Yul `ImmutableFactory` (`0x5FbDB2315678afecb367f032d93F642f64180aa3`) using its signature-authorized `CREATE2` method:

| Contract | Yul Source | Local EVM (localhost) Address | Description |
| :--- | :--- | :--- | :--- |
| **ImmutableFactory** | [immutableFactory.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/immutableFactory.yul) | `0x5FbDB2315678afecb367f032d93F642f64180aa3` | Singular CREATE2 deployment factory. |
| **musicMaker** | [musicMaker.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/musicMaker.yul) | `0x8F1ACB68fF904244322E5ecCa636F8Db4C635Db6` | Emulates C64 SID voice frequency registers. |
| **diskSystem** | [diskSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/diskSystem.yul) | `0xa5Ae84ddeE04baEF9b3BB98B03Cc033F68c891a8` | timing (Jiffies), debouncing, and 1541 DOS commands. |
| **genesis** | [genesis.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/genesis.yul) | `0xe81ed8C20D89C0077246746a49fbc31C01817816` | Handles token creation and launch sequences. |
| **acousticOracle** | [acousticOracle.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/acousticOracle.yul) | `0xf7C8a03878Dbf92566a0BA534e12441e0de81160` | Oracle runtime state and authorized data provider root updates. |
| **graphicsSystem** | [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul) | `0x23a6125dC6D066Ca6E0d5929F6E51BA0232FBAA2` | Emulates VIC-II Sprite register layout and collisions. |
| **cpu6502** | [cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul) | `0x116D694017e4A037a1d62141719a4f8527b65d9c` | Emulates 6502 CPU register operations and opcodes. |

Deployment run via:
```bash
node scripts/deploy_yul.js
```
The addresses are automatically registered in [user_config.json](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/config/user_config.json).

---

## 6. DATAMOST Web Simulation Console

We built an interactive browser-based visualizer and synthesizer dashboard at [frontend/datamost.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/datamost.html):

1. **HTML5 Canvas screen**: Connects to `graphicsSystem` to poll active sprite coordinates and render Sprite 0 (Space Invader) and Sprite 1 (Ghost) on a 320x200 pixel layout with live bounding box collision boxes.
2. **SID Web Audio Oscillator**: Queries frequencies from the `musicMaker` contract and dynamically schedules pitch changes on a Web Audio API sawtooth oscillator.
3. **Register Inspector & Opcode Executor**: Allows developers to view `cpu6502` registers (`A`, `X`, `Y`, `SR`) and execute instructions (`LDA #`, `STA`, etc.) directly on-chain through their browser wallet.

To view the console:
1. Ensure the development server is running (`node scripts/server.js`).
2. Navigate to `http://localhost:3000/datamost.html`.


