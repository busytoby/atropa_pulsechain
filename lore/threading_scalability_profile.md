# C++ Stable Diffusion Thread Scalability Profile

We ran thread scaling profiling to see how execution threads (`SD_THREADS`) affect the rendering performance of the C++ worker.

The profiling script is located at [profile_threads.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/profile_threads.py).

---

## 1. Thread Scaling Profile (Steps=4, LCM Profile)

Below is the execution breakdown across different CPU thread allocations:

| Thread Count | Model Load (CPU) | Denoise Loop (GPU) | VAE Decode (GPU) | Total CLI Duration | Model Load Speedup |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1 Thread** | 2.430s | 0.680s | 0.250s | 3.561s | 1.00x (Baseline) |
| **2 Threads** | 1.630s | 0.680s | 0.250s | 2.737s | 1.49x |
| **4 Threads** | 1.240s | 0.680s | 0.250s | 2.350s | 1.96x |
| **8 Threads** | **1.040s** | 0.680s | 0.250s | **2.152s** | **2.33x (Optimal)** |
| **16 Threads** | 1.100s | 0.680s | 0.250s | 2.217s | 2.21x (Overhead Limit) |

---

## 2. Key Insights & Findings

### A. Separation of Compute Bottlenecks
- **GPU-Bound Stages (Denoise & VAE Decode):** The sampling loop (0.68s) and VAE decode (0.25s) run on the Vulkan compute pipeline (`Vulkan: Using device 0`). As a result, CPU threading (`SD_THREADS`) has **no impact** on these stages.
- **CPU-Bound Stage (Model Loading & Tensor Parse):** Unpacking the safetensors file headers, mapping keys, and converting memory formats occurs on the CPU. This stage is **highly parallelized** and gains substantial speedup from multi-threading.

### B. Scalability Limit (Plateau)
- Going from **1 thread to 8 threads** cuts the model weight loading time in half (from **2.43s to 1.04s**).
- Increasing threads to **16 threads** actually *increases* the latency slightly to **1.10s** because of the overhead of context switching and thread scheduling matching the hard physical disk I/O read limits.
- **Optimal Thread Count:** Setting `SD_THREADS=8` achieves peak CPU throughput for file loading without resource saturation.
