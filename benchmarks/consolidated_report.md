# Consolidated Benchmark Report

* **Date**: 2026-08-05
* **Target Environment**: Linux Native (march=native, -O3, Parallelized via CCX Thread Pool, WinchesterMQ Alignment Enabled, Cache-Aligned Lock-Free Benchmarks)

## 1. Core Component Metrics

### Aho-Corasick Wavelet Arena
* **Memory footprint**: 218,566,272 bytes (208.44 MB)
* **Build time**: 199,737,128 ns
* **Search latency**: 282,519,937 ns

### Headless Vulkan Teddy Renderer
* **Throughput**: 6,570.93 FPS
* **Frame latency**: 0.000152 seconds/frame

### Fast AVX-512 Yang Waveform Synthesis
* **Throughput**: 28,225.79 M-Waveforms/sec
* **Execution duration**: 354.29 ms

### Helmholtz Merkle Level 10 Reduction
* **Throughput**: 4,502.40 reductions/sec
* **Memory scaling density**: 1,125.60 MiB/sec reduced

### DeepSeek-Coder-V2 MLA/MoE Kernels
* **Throughput rate**: 84.67 Million Passes/sec
* **Compute bandwidth**: 10.75 GFLOPS/sec

## 2. Architectural Comparison & Overhead

### Architectural Supremacy (Already There vs. Directed)
* **Directed (standard pointers)**: 19.27 GB/s
* **Field (hardware offsets)**: 29.03 GB/s
* **Relative performance gain**: 1.51x

### Zero-Overhead Paradigms
* **Lock-Free SPSC**: 181.11 Mops/s
* **Zero-Syscall SHM**: 181.11 Mops/s
* **Zero-Copy swap latency**: 0.02 ms
* **Zero-Polling wake latency**: 61.80 us

## 3. Mainframe & Transactional Verification

### JCL Spool Scheduler & COND Verification
* **Spool throughput**: 108,355.86 dispatches/sec
* **COND/Symbol latency**: 4.61 us
* **WinchesterMQ IOPS**: 541,779.31 trans/sec

### CICS Storage Queues
* **Queue throughput**: 118,976.02 ops/sec
* **Commit/Access latency**: 8.41 us
