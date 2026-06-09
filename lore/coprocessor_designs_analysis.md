# Low-Level Coprocessor Designs & On-Chain Register Mapping

In early microcomputer design, the central processor (such as the 8-bit **MOS 6502**) was highly constrained. Running at ~1 MHz with no native support for floating-point operations, hardware multiplication, division, or high-speed memory block moves, performance suffered. 

To overcome these constraints, hardware engineers offloaded computationally expensive tasks to specialized **coprocessors** or custom auxiliary chips.

This document details historic coprocessor architectures and analyzes the virtual coprocessors mapped into our Yul-based [cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul) emulator.

---

## 1. Historic Coprocessor Architectures

### 1.1 MOS 8726 RAM Expansion Unit (REU)
The Commodore REU bypassed the CPU entirely to execute high-speed **Direct Memory Access (DMA)** transfers. 
* **Mechanism:** When a transfer was triggered, the REU asserted the CPU's `DMA` pin, halting the 6510 processor and seizing control of the address and data buses.
* **Speed:** The REU transferred bytes at 1 byte per cycle (approx 1 MB/s), whereas a software CPU copy loop required at least 8 cycles per byte. This was used for fast screen buffering and page swapping.

### 1.2 Math & Geometry Coprocessors
* **Intel 8087 (FPU):** The classic floating-point coprocessor introduced for the 8086/8088. It added 80-bit wide registers and hardware floating-point instructions, executing calculations up to 100x faster than software emulation.
* **Amiga Blitter & Copper:** The Blitter offloaded geometric drawing, line rendering, and rectangular area copies, while the Copper (a co-processor synchronized to the video beam) modified register values mid-scanline without CPU overhead.

---

## 2. On-Chain Virtual Coprocessors inside `cpu6502.yul`

In our on-chain 6502 emulator, complex operations are offloaded to specialized Yul helper blocks mapped to specific memory addresses. These act as virtual coprocessors:

```
  6502 Instruction --> Write to $D6xx Register --> EVM Intercepts Write
                                                            |
                                                   [Yul Coprocessor]
                                                            |
  6502 Instruction <-- Read from $D6xx Register <-- Stores Result
```

### 2.1 COMTAX (Hometax Math Coprocessor)
* **Trigger Address:** `$D6A5` (54949) for calculation; `$D6AA` (54954) for payment release.
* **Math Offload:** Calculates tax bracket thresholds on net assets:
  * Below 100: 0% tax.
  * 100 to 500: 5% tax.
  * Above 500: 10% tax.
* **Register Mappings:** 
  * Inputs: `$D6A1`–`$D6A4` (Gross/Deductions).
  * Outputs: 32-bit tax result mapped to `$D6A6`–`$D6A9` (54950–54953).

### 2.2 Speech Phoneme Synthesis Coprocessor
* **Trigger Address:** `$D400` (54272).
* **Synthesis Offload:** Bypasses slow 6502 speech synthesis software. When a phoneme code is written to the register, the coprocessor translates it to Web Audio format parameters and charges a 1 OTRT Speech Tax.

### 2.3 Wireframe Geometry Coprocessor
* **Trigger Address:** `$D540` (54592).
* **Render Offload:** Coordinates 3D line drawing, projection, and translation. When triggered, it calculates the wireframe points, applies a 1 OTRT wireframe tax, and populates the video RAM page buffers.

---

## 3. Benefits of Virtual Coprocessor Patterns in EVM

By mapping high-level algorithms to hardware register writes, we achieve several benefits:
1. **Gas Optimization:** Complex math (such as 32-bit division or bracketed logic) is executed in native EVM opcodes via Yul helpers, costing far less gas than executing hundreds of emulated 6502 assembly cycles.
2. **Simplified 8-bit Code:** The guest assembly programs remain small and readable, loading inputs into registers and reading outcomes with single instructions.
3. **Dynamic Taxation Enforcement**: Allows the platform to excise transaction taxes (Diyat / OTRT fees) at the exact moment a high-performance coprocessor hardware function is invoked.
