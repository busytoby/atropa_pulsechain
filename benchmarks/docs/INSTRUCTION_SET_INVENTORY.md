# Project Mariner: Instruction Set Inventory (Corrected)
**Date:** 2026-02-07
**Hardware:** Ryzen 9 9950X3D + RX 9070 XT
**Status:** ARCHITECTURE DISTINCTION VERIFIED

## 1. CPU SIMD Architectures (x86-64)
The CPU executes **Vector Instructions** on single threads.

| Width | ISA | Register | Lanes (FP32) | Status |
|---|---|---|---|---|
| **64-bit** | GPR / MMX | `rax`/`mm0` | 2 (Int) | [x] Verified |
| **128-bit** | SSE 4.2 | `xmm0` | 4 | [x] Verified |
| **256-bit** | AVX2 | `ymm0` | 8 | [x] Verified |
| **512-bit** | AVX-512 | `zmm0` | 16 | [x] Verified |

## 2. GPU Wavefront Architectures (RDNA 4)
The GPU executes **Wavefront Instructions** across multiple threads in lock-step.

| Wave Mode | ISA Type | Thread Count | Effective Width | Status |
|---|---|---|---|---|
| **Wave32** | Native RDNA | 32 Threads | 1024-bit total | [x] Verified |
| **Wave64** | Legacy/Wide | 64 Threads | 2048-bit total | [x] Verified |

## 3. Advanced GPU Matrix Extensions (WMMA)
The user has indicated higher-order "Wave" concepts likely map to **Matrix Core** throughput.
*   **Matrix 16x16:** RDNA 3/4 WMMA instructions process 256 elements per operation.
*   This represents a "Logical Wave256" capability in terms of throughput, though physically executed by Wave32/64 units.

**Correction:** Previous reports incorrectly labeled CPU AVX extensions as "Wave" instructions. This distinction is now enforced.
