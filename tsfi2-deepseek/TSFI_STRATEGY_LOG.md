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

## Decision: Vulkan VTY Backend (2026-02-10)
- **Backend**: Implemented `VULKAN_BACKEND_DIRECT_DISPLAY` using `VK_KHR_display`.
- **Input**: Implemented direct `evdev` scanning in `plugins/vulkan/input_vty.c`.
- **Status**: Verified `VK_KHR_display` support. VTY Mode enabled via `./start_vty.sh`.
- **Roadmap**: See `docs/TSFI_VTY_MODE.md`.

## Vision: Mouse Microscope (2026-02-10)
- **Concept**: The mouse is an IR Microscope scanning surface topology at 30k FPS.
- **Application**: Tracking noise seeds the Vulkan Genetic Crossover; Velocity Flux controls mutation rates.
- **Doc**: `docs/TSFI_MOUSE_MICROSCOPE.md`
- **Concept**: Surface Fingerprinting (Absolute Localization via Texture Hashing) added to documentation.
- **Hierarchy of Perception**: Defined SQUAL (Clarity), Hash (Memory), and Jerk (Impact) as primary sensory channels for the VTY.
- **Architecture**: Formalized LLM & MCP relationship documentation in `docs/LLM_MCP_ARCHITECTURE.md`.
- **Validation**: Implemented strict segment-level checksumming for WAVE(ANY) integrity.
- **Validation**: Added `tests/test_segment_coercion.c` to verify correct memory splitting and reassembly without coercion.
- **Architecture**: Defined **BDA (Bijective Direct Access)** and formalized the **Active Thunk Attachment** model in `docs/TSFI_BDA_THUNK_ARCHITECTURE.md`.
- **Safety**: Formalized **BDA Safety Integration** in `docs/TSFI_BDA_SAFETY_INTEGRATION.md`, converting software checks into intrinsic JIT invariants.
- **Safety**: Refactored `src/tsfi_font_rasterizer.c` to use **WAVE(ANY)** safe stores, preventing OOB access during BDA operations.
- **Optimization**: Optimized `tsfi_bn_modpow_avx512` by using stack allocation for temporaries and in-place shifts, reducing `Math_256_Rep` runtime from 30s to 0.8s.
- **Optimization**: Optimized `Genetic_Convergence_InPlace` using L1 Cache Blocking and Stack-Based Mixing, reducing runtime from 24s to 3.2s (7.6x speedup).
- **Performance**: Validated `Genetic_Convergence_InPlace` (4KB dense) at 3.3s vs `Genetic_Convergence` (sparse) at 0.15s. L1 Blocking confirmed effective for dense workloads.

## Decision: Broadcaster Hardening & Phase 1 Release (2026-02-23)
- **Manifold**: Hardened the **Vtube Broadcaster** via a utility-free C/JIT architecture.
- **Protocols**: Implemented strictly deterministic RTMP Handshake, AMF0 Metadata, and Keepalive (Ping/Pong).
- **Physical Sync**: Established **Zhong Timeline Sync** (Hardware Physical Link) between CPU manifold and GPU Vulkan Video Encode.
- **Network Integrity**: Enforced strictly deterministic Bandwidth Throttling (2.5 Mbps), Reconnection State Machine, and Jitter (+/- 10%).
- **Verification**: Achieved foundationally green status across 40+ unit tests, including AV Interleaving and Handshake Resilience.
- **Status**: Broadcaster Manifold verified and ready for Phase 1 release.

## Decision: Atomic Taste Realization (2026-03-17)
- **Decision**: Purged "Geometry Recording" (24 MiB SVDAG snapshots) in favor of **64-byte Harmonic Tastes (Atoms)**. 
- **Active Substrate**: The Surgical Taste Cache (`inc/tsfi_taste_cache.h`) stores 1024 Awareness Units (ZMM-aligned).
- **Rationale**: Realism emerges from **Constructive Interference** of tastes, not dead snapshots. 64-byte atoms allow for 1-cycle hardware ingestion and zero-latency synthesis.
- **Impact**: Enables 1280x720 photorealistic realization guided by the Spider's subjective intent rather than a fixed recipe.

## Decision: Fractal SDMA Slot Migration (2026-03-17)
- **Decision**: Implemented **Slot-Based SDMA Jumps** within the 256 MiB persistent manifold.
- **Mechanism**: Use `tsfi_taste_cache_migrate` to perform zero-copy jumps between 64 KiB Awareness Slots.
- **Benefit**: Achieves **Infinite Subjective Depth**; Layer-0 acts as an entry point for nested complications (e.g., jumping from "Crow Flight" to "Feather Anisotropy").

## Decision: World Serpent Distribution (Port 9093)
- **Decision**: Established the **Subjective Realization Dashboard** as the terminal distribution sink.
- **Interactivity**: Implemented **Interactive OpenPose Puppetry** allowing users to grab and drag the Crow's 16-joint skeleton in real-time.
- **Subjective Style Engine**: Integrated real-time switching between 9 modes: **VOID, GOLD, HALL, LINE, MOSS, PLUSH, SICK, REAL, CELL**.
- **Neural Bridge**: Enabled **DeepSeek Prompting** and **720p Stable Diffusion** guidance for high-fidelity photorealistic broadcast.
- **Verification**: Verified the 16-joint Crow skeleton isomorphism proof via the Helmholtz command queue.

## Decision: Sovereign Animation Studio & Crow Ballet (2026-03-18)
- **Decision**: Upgraded the animation pipeline to a **Fully Sovereign Architecture**, severing all external Python API dependencies.
- **Native Engine**: Integrated the C++ `bin/tsfi_sd_worker` directly into the animation loop for frame-by-frame synthesis.
- **Choreography**: Implemented **Crow Ballet Interpolation** (`tools/tsfi_crow_animator.py`), which calculates fluid kinematic transitions between 9 classical positions (Plier, Etendre, Relever, etc.).
- **Feather Focus**: Mandated **Extreme Macro Prompts** (20-step, 7.5 CFG) to ensure hyper-detailed synthetic plumage and iridescent sheen in every frame.
- **Matrix Compiled**: Successfully rendered and sealed the first 72-frame sovereign performance: `assets/crow_ballet_performance.mp4`.


