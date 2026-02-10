# Wave 512 Mnemonic Review (AVX-512)

This document catalogs the standard Wave 512 assembly mnemonics (AVX-512 instructions) discovered within the `src/tsfi_opt_zmm.c` (ZMM System) and `src/lau_thunk.c` (Thunk JIT System) components.

## 1. ZMM System (`src/tsfi_opt_zmm.c`)
The ZMM system utilizes C intrinsics which map directly to the following mnemonics:

### Memory Operations
-   `VMOVUPS` (via `_mm512_load_ps`, `_mm512_store_ps`): Unaligned load/store of packed single-precision floats.
-   `VMOVNTPS` (via `_mm512_stream_ps`, `_mm512_stream_si512`): Non-temporal (streaming) store of packed data, bypassing cache.
-   `VPBROADCASTQ` (via `_mm512_set1_epi64`): Broadcasts a 64-bit integer to all elements of a vector.

### Arithmetic & Logic
-   `VPADDQ` (via `_mm512_add_epi64`): Packed addition of 64-bit integers.
-   `VPSRLQ` (via `_mm512_srli_epi64`): Packed logical shift right of 64-bit integers.
-   `VPXORQ` (via `_mm512_xor_si512`): Packed bitwise XOR of 64-bit integers.
-   `VPROLQ` (via `_mm512_rol_epi64`): Packed rotate left of 64-bit integers.

## 2. Thunk JIT System (`src/lau_thunk.c`)
The Thunk system emits raw machine code bytes corresponding to these mnemonics:

### Data Movement
-   `MOV` (Opcode: `0x48 0xB8...`, `0x48 0xBF...`, `0x49 0xBF...`): Standard 64-bit register loads (RAX, RDI, RSI, RDX, RCX, R8, R9, R15).
-   `VMOVUPS` (Opcode: `0x62 ... 0x10`, `0x62 ... 0x11`):
    -   Load ZMM from memory (`0x10`).
    -   Store ZMM to memory (`0x11`).
    -   Supports relative addressing from R15 base.
-   `VMOVNTPS` (Opcode: `0x62 ... 0x2B`): Non-temporal store from ZMM to memory.
-   `VBROADCASTSS` (Opcode: `0x62 ... 0x18`): Broadcasts a single float to all ZMM elements (emitted in `emit_zmm_fill`).
-   `VMOVD` (Opcode: `0xC4 ... 0x6E`): Moves a 32-bit integer to an XMM register (part of `emit_zmm_fill`).

### Arithmetic
-   `VADDPS` (Opcode: `0x62 ... 0x58`): Packed single-precision floating-point addition (used in `emit_zmm_density_op`).

### Control Flow
-   `JMP` (Opcode: `0xFF 0xE0`): Indirect jump to address in RAX.
-   `JZ` (Opcode: `0x74 ...`): Jump if Zero (conditional relative jump).
-   `CALL` (Opcode: `0xFF 0xD0`): Indirect call to address in RAX.
-   `RET` (Opcode: `0xC3`): Return from function.

### Prefetching
-   `PREFETCHT0` (Opcode: `0x0F 0x18 0x08`): Prefetch data into all cache levels.
-   `PREFETCHT1` (Opcode: `0x0F 0x18 0x10`): Prefetch data into L2 cache and higher.
-   `PREFETCHT2` (Opcode: `0x0F 0x18 0x18`): Prefetch data into L3 cache and higher.

## Summary
The system actively employs a comprehensive set of AVX-512 mnemonics for:
1.  **Massive Data Movement:** `VMOVUPS`, `VMOVNTPS` (Zero Copy focus).
2.  **Vector Arithmetic:** `VPADDQ`, `VPXORQ`, `VADDPS` (Scramble & density kernels).
3.  **Cache Control:** `PREFETCHTx` family (Memory latency hiding).
