# Consolidated Benchmark Report

* **Date**: 2026-08-05
* **Target Environment**: Linux Native (march=native, -O3, Parallelized via CCX Thread Pool, WinchesterMQ Alignment Enabled, Direct Access Thunk Verification, Monte Carlo Guidance Enabled)

## 1. Core Component Metrics

### Aho-Corasick Wavelet Arena
* **Memory footprint**: 218,566,272 bytes (208.44 MB)
* **Build time**: 394,876,510 ns
* **Search latency**: 258,605,848 ns

### Headless Vulkan TeddyBear Renderer
* **Throughput**: 6,544.91 FPS
* **Frame latency**: 0.000153 seconds/frame

### Fast AVX-512 Yang Waveform Synthesis
* **Throughput**: 24,851.43 M-Waveforms/sec
* **Execution duration**: 402.39 ms

### Helmholtz Merkle Level 10 Reduction
* **Throughput**: 4,499.19 reductions/sec
* **Memory scaling density**: 1,124.80 MiB/sec reduced

### DeepSeek-Coder-V2 MLA/MoE Kernels (AVX-512 Optimized)
* **Throughput rate**: 1,501.75 Million Passes/sec
* **Compute bandwidth**: 95.36 GFLOPS/sec

## 2. Architectural Comparison & Overhead

### Architectural Supremacy (Already There vs. Directed)
* **Directed (standard pointers)**: 19.90 GB/s
* **Field (hardware offsets)**: 29.11 GB/s
* **Relative performance gain**: 1.46x

## 3. Zero-Overhead Paradigms (Direct Access Thunks)
* **Lock-Free SPSC**: 4,850.33 Mops/s
* **Zero-Syscall SHM**: 4,850.33 Mops/s
* **Zero-Copy swap latency**: 0.02 ms
* **Zero-Polling wake latency**: 1.52 us

## 4. Mainframe & Transactional Verification

### JCL Spool Scheduler & COND Verification
* **Spool throughput**: 111,035.14 dispatches/sec
* **COND/Symbol latency**: 4.50 us
* **WinchesterMQ IOPS**: 555,175.69 trans/sec

### CICS Storage Queues
* **Queue throughput**: 110,779.65 ops/sec
* **Commit/Access latency**: 9.03 us
