# TSFi Strategy Log

## Decision: Math Library Architecture (2026-02-05)
- **Decision:** The experimental **Wave512** math library (Pure Assembly implementation) has been **archived** to `src/tsfi_math_wave512_archive.c`.
- **Active Library:** The project now uses the **AVX-512** implementation (`src/tsfi_math.c` restored) which utilizes compiler intrinsics (e.g., `_mm512_madd52lo_epu64`) and requires `-mavx512ifma`.
- **Rationale:** The AVX-512 implementation is "extremely faster" and stable.
- **Wave512 Usage:** The Wave512 Mnemonic ISA (`inc/tsfi_wave512.h`) remains active for **Vision** and **Hilbert** components, where "Hardware Transcendence" (Pure ASM) is prioritized over raw integer throughput.
- **Testing:** Unit tests (`tests/test_math.c`) now verify the AVX-512 implementation. The Wave512 math logic is no longer linked or tested.

## Decision: HelmK0Rn Transition
- **Decision:** Legacy components `tsfi_font_engine` and `tsfi_tessellator` are deprecated.
- **Replacement:** The **HelmK0Rn** architecture (`src/tsfi_k0rn_compiler.c`) replaces the old font engine.
- **Testing:** Deprecated tests (`test_font_engine`, `test_tessellator`) have been removed from the active suite.

## Footprint Baseline (2026-02-06)
- **Total Size:** 0 bytes
- **Total Lines:** 0 lines
- **Scope:** tools/gemini-cli-fork/packages src inc

## Footprint Baseline (2026-02-06)
- **Total Size:** 12406652 bytes
- **Total Lines:** 400538 lines
- **Scope:** tools/gemini-cli-fork/packages src inc
- **Artifact:** decis/render_1774048708.mp4 (Seed: 1774048708)
- **Milestone Video:** decis/milestone_20260206_092118.mp4 (Duration: 5s)
- **Artifact:** decis/render_1774049314.mp4 (Seed: 1774049314)
- **Artifact:** decis/render_1774049999.mp4 (Seed: 1774049999)
- **Artifact:** decis/render_1774050011.mp4 (Seed: 1774050011)
- **Artifact:** decis/render_1774050024.mp4 (Seed: 1774050024)
- **Artifact:** decis/render_1774050781.mp4 (Seed: 1774050781)
- **Artifact:** decis/render_1774050793.mp4 (Seed: 1774050793)
- **Milestone Video:** decis/milestone_20260206_093030.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774051587.mp4 (Seed: 1774051587)
- **Milestone Video:** decis/milestone_20260206_093457.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774052150.mp4 (Seed: 1774052150)
- **Milestone Video:** decis/milestone_20260206_093802.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774052358.mp4 (Seed: 1774052358)
- **Milestone Video:** decis/milestone_20260206_093913.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774054729.mp4 (Seed: 1774054729)
- **Milestone Video:** decis/milestone_20260206_100018.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774055059.mp4 (Seed: 1774055059)
- **Milestone Video:** decis/milestone_20260206_100210.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774056353.mp4 (Seed: 1774056353)
- **Milestone Video:** decis/milestone_20260206_101202.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774056606.mp4 (Seed: 1774056606)
- **Milestone Video:** decis/milestone_20260206_101233.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774056858.mp4 (Seed: 1774056858)
- **Milestone Video:** decis/milestone_20260206_101256.mp4 (Duration: 5.000000s)
- **Artifact:** decis/render_1774057116.mp4 (Seed: 1774057116)
- **Milestone Video:** decis/milestone_20260206_101337.mp4 (Duration: 5.000000s)

## Decision: Bijective Supremacy Realized (2026-02-09)
- **WAVE(Any)**: Achieved variadic atom support via `WaveStream` and strided unrolling.
- **Resonance**: Verified autonomous CPU/GPU path selection in `TSFiOperator`.
- **Integrity**: Implemented context-sensitive Reactive CRC (Dysnomia Ichidai/Daiichi).
- **Physicality**: Shifted from "Holographic" synthetic benchmarks to Physical Reality (0.88ns dispatch).
- **Next**: Scale the Fused Pipeline into complex Dysnomia reactions (SHAO/SHIO/YI).
