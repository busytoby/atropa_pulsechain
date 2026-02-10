# TSFi Decision Testbed Engine
> **Evolutionary Strategy for Safe Refactoring**

This document outlines the design for a "Decision Engine" capable of evaluating architectural changes (refactors) before they are merged into the core lineage. It formalizes the **HelmK0Rn Transition** methodology: *Parallel Implementation → Invariant Verification → Hot-Swap*.

## 1. Core Philosophy: The "HelmK0Rn Pattern"
Refactoring in TSFi is not about "editing code"; it is about **evolving architecture**.
- **Phase 1 (Incubation):** New logic is implemented in parallel (e.g., `src/tsfi_k0rn_compiler.c`) without touching the live system.
- **Phase 2 (The Decision Testbed):** The new component is subjected to a rigid "Decision Engine" that verifies:
    1.  **Bijectivity:** Does it map 1:1 to `LauRegisterBank`?
    2.  **Zero-Copy:** Does it use `_mm512_stream` / `lau_malloc_wired`?
    3.  **Performance:** Does it meet/exceed current throughput?
- **Phase 3 (Hot-Swap):** If Approved, the `lau_wire_system` pointers are re-bound to the new logic. Legacy code is deprecated.

## 2. Decision Engine Architecture

### A. Inputs
- **Candidate Source:** The new implementation (e.g., `src/tsfi_math_new.c`).
- **Baseline Source:** The current "Champion" (e.g., `src/tsfi_math.c`).
- **Invariant Suite:** A specialized test harness for architectural rules.

### B. The Engine (`bin/tsfi_decision`)
A meta-tool that automates the evaluation:
1.  **Static Analysis:**
    -   Greps for banned tokens (e.g., `malloc` instead of `lau_malloc`).
    -   Verifies `tsfi_wave512.h` usage over raw scalar loops.
2.  **Dynamic Verification:**
    -   Compiles Candidate in a sandbox.
    -   Runs `tests/test_invariants.c` against the Candidate.
3.  **Benchmark Duel:**
    -   Runs `Candidate` vs `Baseline` in a 10s throughput race.

### C. Outputs (The Verdict)
- **[ACCEPTED]**: Candidate is faster and compliant. Auto-merge strategy.
- **[REJECTED]**: Candidate failed invariants or was slower.
- **[ARCHIVE]**: Candidate is valid but not currently needed (saved to `archive/`).

## 3. Implementation Roadmap

1.  **`tests/test_invariants.c`**: Create a universal test that checks any object for:
    -   512-byte Alignment.
    -   Header Integrity (`LauWiredHeader`).
    -   Thunk Table validity.
2.  **`scripts/run_decision.sh`**: The automation script.
    -   `./scripts/run_decision.sh --candidate src/new_math.c --baseline src/math.c`
3.  **Integration**: Link into `start_gemini.sh` as `--propose`.

## 4. Example: Future Refactor (Math Library)
Instead of deleting `tsfi_math.c`, an agent would:
1.  Create `src/tsfi_math_v2.c`.
2.  Run `decision`.
3.  If `v2` is superior, the Engine updates `TSFI_STRATEGY_LOG.md` and swaps the file.
