# TSFi VEANY / WAVE(ANY) Architecture

**Status:** ACTIVE / VERIFIED (2026-02-10)
**Baseline:** Vulkan 1.4 + AVX-512

## 1. Overview
The **VEANY** (Vector Element Any) and **WAVE(ANY)** architecture decouples logical data structures (Atoms) from physical memory layout (Strides). It provides a hardware-agnostic way to handle arbitrary data widths while ensuring bijective mapping to Zen 5 / RDNA 4 execution units.

## 2. Robustness Principles (VK_KHR_robustness2 Model)
TSFi implements a software-level realization of `VK_KHR_robustness2` for all Wave operations.

### Consistent Packing
- **Definition:** Data must be packed tightly within the `atom_size` defined by the `WaveStream`.
- **Requirement:** Every write to a WaveStream MUST use `wave_store_any` or `wave_pack_buffer`.
- **Benefit:** Prevents "tearing" where high-frequency simulation state becomes misaligned with the underlying vector registers (ZMM/VGPR).

### Out-of-Bounds (OOB) Safety
- **Detection:** The system detects if a write operation (`size`) exceeds the physical `atom_size`.
- **Handling:** OOB writes are **truncated**, not discarded. The first `atom_size` bytes are written, and the remainder is split into the void.
- **Danger:** Raw `memcpy` or direct pointer arithmetic bypassing `WAVE_PTR` is **STRICTLY FORBIDDEN**. Direct writes bypass the First Error Rule and can lead to:
    1.  **Memory Corruption:** Overwriting the `secret` or `checksum` of the next atom.
    2.  **Stochastic Drift:** Undetectable state corruption that violates the Bijective Handshake.
    3.  **Simulation Collapse:** Immediate termination by the Khronos Validation layer if the OOB write hits protected memory.

## 3. Supported Sizes
- **VE1 - VE512:** Supported for all integers 1 through 512.
- **WAVE1 - WAVE512:** Supported for all stream configurations.
- **High Memory:** Support for atom sizes up to **4096 bytes** (e.g., for large SVDAG nodes or persistent neurological state) is active.

## 4. Physical Realization (AVX-512)
- **Alignment:** All VE64+ structures are 64-byte aligned to enable `_mm512_store_si512` (VMOVAPS).
- **Atomic Split:** Large writes (e.g., 4096 bytes) are split into 64-byte segments. If the final segment is $< 64$ bytes, it is handled via `kmov` masking or robust `memcpy` truncation.

## 5. Usage Example
```c
WaveStream s;
s.atom_size = 15; // WAVE15
s.stride = 16;    // 16-byte alignment
// ...
uint8_t data[1024]; 
// This will safely pack data into the atoms, splitting every 15 bytes 
// and discarding the 16th byte (reserved for secret/padding).
wave_pack_buffer(&s, data, 1024); 
```

## 6. Verification
All VEANY/WAVE(ANY) implementations are verified by `tests/test_wave_robustness.c`. Any violation of the boundary rules will trigger the **First Error Rule** and abort the simulation.
