# Project: Automated Benchmark Suite and Visual Report Generator

## Architecture
- Benchmarks Directory: `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite`
- Components to profile:
  - Teddy Bear Editor (Vulkan/Wayland tool palette with bear mesh and interactive sliders)
  - Drum Sequencer (Rhythmic Bits step sequencer or the built-in sequencer UI)
- Key metrics: FPS, loop iteration time, ALSA audio latency (time from sound trigger to ALSA playback output)
- Output format: JSON or CSV data files.
- Visual Report: HTML containing SVG or Canvas graphs.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | Exploration | Locate binaries, understand ALSA implementation and rendering loops | None | DONE |
| 2 | Benchmark Engine | Develop CLI benchmarks script, add timing/telemetry hooks to binaries | M1 | DONE |
| 3 | Visual HTML Reporter | Develop generator script for interactive SVG/Canvas charts in standalone HTML | M2 | DONE |
| 4 | Verification & Hardening | Run end-to-end, profile metrics, check HTML loading, perform forensic audit | M3 | DONE |

## Interface Contracts
- Telemetry Output: JSON containing `{ "fps": [...], "loop_time_ms": [...], "audio_latency_ms": [...] }` or CSV.
- HTML Visualizer: Reads the generated JSON/CSV, injects/embeds the data, and renders interactive SVG/Canvas plots.
