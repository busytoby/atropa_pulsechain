# Consolidated Benchmark Report

* **Date**: 2026-08-05
* **Target Environment**: Linux Native (march=native, -O3, Parallelized via CCX Thread Pool, WinchesterMQ Alignment Enabled)

## 1. Core Component Metrics

### Aho-Corasick Wavelet Arena
* **Memory footprint**: 218,566,272 bytes (208.44 MB)
* **Build time**: 412,396,954 ns
* **Search latency**: 273,552,991 ns

### Headless Vulkan Teddy Renderer
* **Throughput**: 6,572.97 FPS
* **Frame latency**: 0.000152 seconds/frame

### Fast AVX-512 Yang Waveform Synthesis
* **Throughput**: 28,687.68 M-Waveforms/sec
* **Execution duration**: 348.58 ms

### Helmholtz Merkle Level 10 Reduction
* **Throughput**: 4,479.13 reductions/sec
* **Memory scaling density**: 1,119.78 MiB/sec reduced

### DeepSeek-Coder-V2 MLA/MoE Kernels
* **Throughput rate**: 225.47 Million Passes/sec
* **Compute bandwidth**: 28.63 GFLOPS/sec

## 2. Architectural Comparison & Overhead

### Architectural Supremacy (Already There vs. Directed)
* **Directed (standard pointers)**: 19.88 GB/s
* **Field (hardware offsets)**: 28.71 GB/s
* **Relative performance gain**: 1.44x

### Zero-Overhead Paradigms
* **Lock-Free SPSC**: 241.43 Mops/s
* **Zero-Syscall SHM**: 241.43 Mops/s
* **Zero-Copy swap latency**: 0.02 ms
* **Zero-Polling wake latency**: 61.75 us

## 3. Mainframe & Transactional Verification

### JCL Spool Scheduler & COND Verification
* **Spool throughput**: 105,592.52 dispatches/sec
* **COND/Symbol latency**: 4.74 us
* **WinchesterMQ IOPS**: 527,962.59 trans/sec

### CICS Storage Queues
* **Queue throughput**: 113,927.48 ops/sec
* **Commit/Access latency**: 8.78 us
