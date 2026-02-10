# RDNA4 Font Engine & AI Improvements

## 512-bit Bijective Alignment
The TSFi Font Engine has been optimized for **RDNA4 Workgroup Processors (WGP)** and **Zen 5 (AVX-512)** CPU architectures. The core of this optimization is the 512-bit alignment of the `LauWiredHeader` and associated Hilbert fields.

### Key Hardware Features
*   **16-Wide Float32 SIMD:** Both Zen 5 and RDNA4 (in Wave32 mode) process 512-bit chunks of data natively. Our `tsfi_hilbert_batch_avx512` implementation utilizes ZMM registers to evaluate 16 points of the Hilbert field in parallel.
*   **SGPR Residency:** By mapping the `TSFiHilbertGlyph` matrix (289 coefficients) directly into the `LauRegisterBank` (simulated SGPRs), we achieve zero-latency constant access for the phase-space evaluation loop.
*   **WMMA Compatibility:** The separable nature of the Hilbert evaluation ($V = E_x \cdot C \cdot E_y^T$) is designed for **Wavefront Matrix Multiply Accumulate** instructions, targeting Giga-Point per second throughput.

## Thunk-Based Function Inheritance
All AI workloads (Evolve, Recognize, Collapse) are implemented via JIT-compiled assembly thunks stored in the `LauWiredHeader`. 

### Assembly Pattern (`ThunkProxy_emit_mixed`)
```nasm
mov %rdi, %rsi    ; Shift intensity to Arg2
movabs $ctx, %rdi ; Inject Baked Context into Arg1
movabs $fn, %rax  ; Load Target
jmp *%rax         ; Tail call
```
This pattern allows Font instances to "inherit" AI behaviors from parents by cloning the thunk pool, or "override" them by rebinding the jump target, all without modifying the core engine code.

## Zero-Indirection SGPR Mapping
To further minimize latency, the `LauRegisterBank` (simulated SGPRs/VGPRs) is now **directly embedded** within the `LauWiredHeader`. 

### Architectural Impact
*   **Elimination of Pointer Dereference:** Previously, accessing SGPR coefficients required traversing `h->regs->sgpr`. The new architecture allows direct field access (`h->regs.sgpr`), removing a memory load from the execution critical path.
*   **Cache Affordance:** The entire header, including the "Glyph DNA" (coefficients), now resides in a single contiguous block of cache-aligned memory. This increases the likelihood of the coefficients being pre-fetched alongside the system metadata.
*   **Bijective Sealing:** The embedded SGPRs are now included in the `LauWiredHeader` checksum, ensuring that any tampering with the glyph's spectral representation will invalidate the object's seal.

## Performance Metrics
*   **Hilbert Evaluation:** ~38 MPoints/sec (Single-threaded AVX-512).
*   **Font Rasterization:** ~304k Glyphs/sec (AVX-512 BDA).
*   **Driver Overhead:** ~0.1 ms (Vulkan Dispatch).