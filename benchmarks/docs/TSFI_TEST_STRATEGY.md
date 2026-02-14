# TSFi Testing Strategy & Optimization

## 1. Overview

The TSFi test suite is designed for **Maximum Processor Utilization** and **Bijective Verification**. Unlike traditional CI/CD pipelines that run tests sequentially or with limited concurrency, TSFi employs a "Flood Strategy" to saturate all available CPU cores, validating not just logic correctness but also system stability under load (AddressSanitizer/ThreadSanitizer stress).

## 2. Test Runner Architecture (`test_all.sh`)

The runner operates in a **Global Parallelism** mode.

*   **Launch Phase:** All test binaries are spawned immediately as background processes (`&`).
*   **Time Boxing:** A strict `timeout` (via `tsfi_time`) enforces latency bounds. Tests that hang or contend for locks too long are killed (Exit 124).
*   **Sanitization:** All critical kernels are compiled with `-fsanitize=address,undefined` (ASan/UBSan) to catch memory corruption and undefined behavior in real-time.
*   **Dependency Management:** The runner explicitly links object files (`obj/*.o`) rather than recompiling sources, ensuring consistency with the main build.

## 3. Modes of Operation

| Mode | Flag | Timeout | Purpose |
| :--- | :--- | :--- | :--- |
| **Fast** | Default | 48s | Developer iteration. Sanity checks. |
| **Thorough** | `--thorough` | 98s | Pre-commit validation. Full benchmarks. |
| **Full** | `--full` | 286s | Release certification. Stress testing. |

## 4. Key Test Categories

### A. Neurological & Math Kernels (High Priority)
*   `test_math`: Validates AVX-512 BigInt arithmetic (256-bit to 4096-bit).
*   `bench_neurology`: Stress tests the Thetan (YANG) transformations.
*   `test_wave_split_32`: Verifies atomic wave operations.

### B. Genetic & Evolutionary
*   `genetic.establish_llm`: Verifies the RPC ability to bootstrap internal models.
*   `test_genetic_crossover`: Validates the `Fourier_UniversalCrossover` logic.

### C. System & Memory
*   `test_allocator`: Checks `lau_malloc_wired` alignment and scrambling.
*   `test_zero_copy_switch`: Verifies context switching latency.

### D. Vision & Vulkan
*   `test_gpu_cache_packing`: Checks data alignment for RDNA 4 caches.
*   `test_vulkan_lifecycle`: Mocks the display server interaction.

## 5. Optimization & Redundancy Policy

*   **Redundancy:** Tests must not duplicate logic. `bench_` files should focus on throughput/latency, while `test_` files focus on correctness.
*   **Full Utilization:** The suite uses `run_parallel` for *everything*. We do not throttle. If the system crashes, the code is not robust enough.
*   **Object Reuse:** We link against `obj/` files to avoid redundant compilation during the test phase, saving seconds per run.

## 6. Debugging Failures

*   **Timeouts (Exit 124):** The test is too slow or deadlocked. Check lock hierarchy.
*   **Linker Errors:** Ensure the test compilation command in `test_all.sh` includes all necessary `-l` flags (e.g., `-lm`) and object dependencies.
*   **ASan Leaks:** Use `ASAN_OPTIONS=detect_leaks=0` only if the leak is known/benign (e.g., driver initialization). Ideally, fix the leak.
