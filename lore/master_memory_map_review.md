# The On-Chain "Master Memory Map" (C64 & Yul MMIO Spec)

In Commodore 64 systems, the *Master Memory Map* served as the developer bible, detailing every byte from zero page pointer structures to specialized hardware registers. In our decentralized environment, the Yul emulator ([cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul)) translates this classic memory map into blockchain actions, treating external contract calls, state storage, and telemetry as physical peripheral hardware.

---

## 1. Core Virtual Memory Allocation

| Memory Range (Hex) | Memory Range (Dec) | Traditional C64 Function | Virtualized On-Chain Mapping / EVM Action |
| :--- | :--- | :--- | :--- |
| **`$0000 - $00FF`** | `0 - 255` | Zero Page | Fast-access storage variables; mapped directly to guest execution memory |
| **`$0100 - $01FF`** | `256 - 511` | CPU Stack | Stack pointer boundaries managed inside `cpu6502.yul` |
| **`$0400 - $07E7`** | `1024 - 2023` | Screen VRAM | Framebuffer text matrix parsed by the terminal visual renderer |
| **`$0800 - $9FFF`** | `2048 - 40959` | BASIC Program Area | RAM storage containing compiled 6502 execution bytes |
| **`$A000 - $BFFF`** | `40960 - 49151` | BASIC ROM / Cartridges | Mapped to cartridge ROM states (e.g., *Insta-Calc* cartridge memory) |
| **`$B600 - $B6FF`** | `46592 - 46847` | Free Space | **Disk Controller Space** (Atropa ISA: Track, sector, and R/W flags) |
| **`$D000 - $D02E`** | `53248 - 53294` | VIC-II Video Chips | Horizontal/vertical raster synchronization and collision registries |
| **`$D500 - `$D52F`** | `54528 - 54575` | Custom Expansion | **MMIO Device Space** (Oracle data queries, ERC-20 token triggers) |
| **`$D600 - $D60F`** | `54784 - 54799` | Custom Speech | **Speech Synthesizers** (Votrax Phonemes, TMS5220 commands, Speech Tax) |
| **`$D630 - $D650`** | `54832 - 54864` | Custom Modem | **Software to Go Library** (Modem query buffers & download trigger) |
| **`$D800 - $DBE7`** | `55296 - 56295` | Color RAM | High-res text color matrix parsed by the terminal renderer |

---

## 2. Deep Dive: Memory-Mapped I/O (MMIO) Registers

Our `cpu6502.yul` and peripheral modules intercept writes to specific locations to execute smart contract operations:

### 2.1 VIC-II Video Emulation
*   **`$D011` (`53265`)**: Control Register 1. Mapped to check the 9th bit of the virtual scanline target.
*   **`$D012` (`53266`)**: Raster Line Compare. Writing a line index here sets up on-chain vertical synchronization ticks.
*   **`$D019` (`53273`)**: Interrupt Flag Register. Clearing raster interrupt flags via write-one-to-clear filters.

### 2.2 On-Chain Oracle & Payment Gateway (`$D500–$D520`)
*   **`$D500` (`54528`)**: Sensor Input Port. Write updates trigger an EVM `staticcall` to a specified Oracle contract.
*   **`$D510` (`54544`)**: Payment Output Register. Triggers token validation and transfer routines.
*   **`$D520` (`54560`)**: Entropy Generator. Intercepting reads generates on-chain randomness using `keccak256(blockhash, timestamp)`.

### 2.3 Synthesizer Audio Spaces (`$D600–$D609`)
*   **`$D600` (`54784`)**: Votrax Phoneme register. Writing here triggers the virtual speech output engine, deducting a **Diyat Speech Tax** (1 OTRT token) on-chain.
*   **`$D608 - $D609` (`54792 - 54793`)**: TMS5220 Command/Data interface, mapping to a 16-byte internal FIFO buffer inside the Yul memory layout.

### 2.4 Software to Go Modem Downloader (`$D630–$D650`)
*   **`$D630` (`54832`)**: File Index Query.
*   **`$D631 - $D64F` (`54833 - 54863`)**: Metadata text buffers containing target filenames and sizes.
*   **`$D650` (`54864`)**: Download Trigger. Writing a file index here deducts the **Diyat Library Tax** (2 OTRT) and pulls down file sectors directly into the caller's namespaced disk registry (`diskSystem.yul`).

---

## 3. Benefits of Virtual MMIO Architecture

1.  **Instruction-Set Portability**: Legacy 6502 binaries interact with hardware as if they were running on real silicon, while the underlying Yul engine converts these interactions into secure EVM state updates.
2.  **Explicit Resource Sandboxing**: User RAM registers are completely isolated from peripheral control registers, preventing any malicious overflow of execution boundaries.
3.  **Low-Overhead Call Execution**: Bypasses traditional Web3 JSON-RPC polling by allowing retro hardware loops to query data dynamically using PEEK and POKE instructions.
