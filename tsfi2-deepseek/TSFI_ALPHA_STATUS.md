# TSFi Alpha Science Support Status

**Date:** 2026-02-07
**Engine:** TSFi In-Memory MCP (`tsfi_mcp_server`)
**Status:** ACTIVE / INTEGRATED

## Overview
The TSFi "Project Alpha" support provides high-performance, in-memory implementations of biological and algorithmic evolution primitives, powered by the AVX-512 Zero-Copy stack.

## Components

### 1. AlphaFold (Structural Realization)
**RPC Method:** `alphafold.fold`
**Mechanism:**
1.  **Crossover:** Hybridizes input sequence with a "Fold Potential" template using Vectorized Uniform Crossover.
2.  **Folding:** Converts the resulting DNA into a `TSFiHilbertGlyph` via `tsfi_hilbert_fold_sequence_avx512`.
3.  **Realization:** Compiles the Glyph into a Spatial SVDAG (Sparse Voxel DAG).
4.  **Metric:** Returns the `mass` (integral of Feynman Points) as a stability proxy.
**Performance:** < 100µs per fold.

### 2. AlphaEvolve (Algorithmic Evolution)
**RPC Method:** `alphaevolve.step`
**Mechanism:**
1.  **Selection:** Accepts `algorithm` (Parent A) and optional `parent_b` (Parent B).
2.  **Crossover:** Swaps instructions/operands using Uniform Crossover to generate syntactically diverse progeny.
3.  **Result:** Returns the evolved algorithm string immediately.
**Use Case:** Rapid evolution of `WAVE512` assembly or optimization heuristics.

### 3. AlphaProteo (Protein Design)
**RPC Method:** `alphaproteo.design`
**Mechanism:**
1.  **SVDAG:** Generates a de-novo spatial structure based on `entropy` input.
2.  **Screening:** (Planned) Massively parallel screening of candidates against the generated SVDAG.

## Architecture
- **Zero-Copy:** All data remains in mapped memory; no serialization overhead within the engine.
- **AVX-512:** Genetic and Spatial operations are fully vectorized.
- **Thread-Safe:** Stateless request handling (per-request memory wiring).
