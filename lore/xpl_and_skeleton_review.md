# Review of xpl Systems and SKELETON DAT Structures

This document reviews the architectural specifications, implementations, and data pipelines of the **xplos** (XPL Operating System) active VM bus and the **SKELETON** rendering/skeletonization datasets.

---

## 1. The xplos VM & Compiler Architecture

The [xplos_architecture_manifesto.md](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/lore/xplos_architecture_manifesto.md) establishes a hardware-integrated active bus pipeline consisting of:

*   **SKELETON (Generator):** The compiler code generation layer that maps symbolic inputs to template cards (resolving offsets using MotzkinPrime).
*   **XCOM (Active Bus):** The communication bus handling frame transmission over virtual coaxial paths and SCSI WinchesterMQ queues.
*   **ANALYZER (Auditor):** The static validation layer enforcing column sequence checks, syntax constraints (via minimal non-recursive Backus-Naur Form), and safety boundaries on the RED rail.
*   **XPLSM (VM Monitor):** The execution loop driver checking runtime validation conditions on the BLACK rail.
*   **ALU Semiconductor:** The conduction gating logic layer that routes bus traffic and manages transaction commitments.

---

## 2. SKELETON data pipelines (.dat.bin files)

Per **Rule 13 (Quadtree Media Formatting)**, the system restricts database slices, block-ledgers, and index coordinates to the `.dat.bin` format on disk. No `.json` storage layout is permitted for quadtree media.

*   **Nadler Skeletonizer:** Deployed in [tsfi_nadler_skeletonizer.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_nadler_skeletonizer.c). It uses the Zhang-Suen thinning iteration helper, measures soft-body FET discharge curves under Rule 10, and writes coordinate outputs using the `.dat.bin` layout pattern (e.g., `NADLER_SKELETON_[Pixels].dat.bin`).
*   **TeddyBear Skeleton Pipeline:** Deployed in [tsfi_teddy_bear_skeleton.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_teddy_bear_skeleton.c). It renders kinematic bones and coordinates to shared memory frames, integrating continuous Fourier-G contours for complex models (e.g. `PUPPET_TYPE_CROW`).
*   **Syntax Reduction Tests:** Located in [test_xpl_skeleton.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_xpl_skeleton.c), which tests the parser's shift-reduction steps (`BYTE(address) = value;`) and interacts with the ZMM VM.

---

## 3. Current System Integration Status

All files and pipelines compile and execute as part of the overall test suites:
- Key binary mapping matches the modular structure.
- All disk assets conform to the `.dat.bin` structure to satisfy database and layout rules.
