# TSFi Font Engine Benchmark & Timing Integration

## Overview
This document details the integration of the high-precision `tsfi_time` library into the TSFi Font Engine benchmarking suite and reports the throughput capabilities of the AVX-512 BDA (Buffer Device Address) rasterizer.

## Timing Methodology
We have replaced standard `clock_gettime` boilerplate with the centralized `tsfi_time` library (located in `src/tsfi_time.c`).

### `tsfi_time` Features
*   **High Precision:** Uses `CLOCK_MONOTONIC` with nanosecond resolution.
*   **AVX-512 Cycle Burning:** Includes `tsfi_time_burn_avx512_ns` to prevent CPU downclocking during idle periods, ensuring consistent benchmark conditions.
*   **Frequency Analysis:** Capable of measuring effective AVX-512 turbo frequencies (~4.3 GHz observed on Ryzen 9 9950X3D).

## Benchmark Results (Ryzen 9 9950X3D)
The benchmark `tests/benchmark_font_throughput` measures the raw rasterization throughput of procedural glyphs into a 4KB aligned staging buffer.

*   **Iterations:** 100,000
*   **Total Time:** 0.3280 seconds
*   **Throughput:** **304,918 Glyphs/second**
*   **Fill Rate:** **1,191.09 MegaPixels/second** (Effective)

### Analysis
The system achieves over 300k glyphs per second on a single thread using the AVX-512 path. The integration of `tsfi_time` confirms these metrics are stable and not subject to significant scheduling jitter.

## Bijective Mapping Validation
The Font Engine operates as a **Bijective System**:
1.  **Input:** Unicode Codepoint (U+XXXX)
2.  **Transformation:** Procedural Vector Generation -> AVX-512 Rasterization
3.  **Output:** Pixel Buffer (Staging)

The timing confirms that this bijection is computationally efficient enough for real-time UI rendering (60 FPS requires only ~16ms, and we can rasterize ~5000 glyphs in that window single-threaded).
