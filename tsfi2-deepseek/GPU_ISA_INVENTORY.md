# Project Mariner: Wave512 ISA Instruction Page
**Date:** 2026-02-07
**Status:** FULL ISA CATALOG VERIFIED

## 1. Wave512 Mnemonic Map (Native GPU)
The following mnemonics are certified as part of the physical Wave512 instruction set.

| Mnemonic | Description | Physical Opcode |
|---|---|---|
| **VADDPS** | Wavefront Addition | `vaddps` |
| **VSUBPS** | Wavefront Subtraction | `vsubps` |
| **VMULPS** | Wavefront Multiplication | `vmulps` |
| **VDIVPS** | Wavefront Division | `vdivps` |
| **VFMADD** | Realization FMA | `vfmadd213ps` |
| **VPMADD52**| High-Precision IFMA | `vpmadd52luq` |
| **VPERMPS** | Logical Permute | `vpermps` |
| **VMOVAPS** | Aligned Load/Store | `vmovaps` |
| **VPMAXUB** | Scramble Operator | `vpmaxub` |

## 2. Structural Mandate
*   **Register Clustering:** Wave512 instructions execute across **32 physical ZMM registers** (Total RF saturation).
*   **Alignment:** `__attribute__((aligned(512)))` is the physical entry requirement for this ISA page.
*   **Target:** Native GPU Realization (Zhong Memory Model).

**Certification:** This catalog defines the complete logical-to-physical mapping for the TSFi Wavefront Engine.