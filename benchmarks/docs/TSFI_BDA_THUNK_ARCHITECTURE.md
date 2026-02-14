# TSFi BDA & Thunk Architecture: Beyond Direct Access

## 1. Definition: What is BDA?

**BDA (Big Data Area)**, in the context of high-performance compute and the TSFi ecosystem, refers to a **Bijective Direct Access** memory region.

*   **Traditionally:** It represents a large, contiguous block of unmanaged memory (often Gigabytes) mapped directly into the process address space (e.g., via `mmap` or ReBAR). It is a "dumb" container for raw tensors, textures, or geometry.
*   **The Problem:** Accessing BDA requires the CPU/GPU to "coerce" this raw data into structured types at runtime. This introduces latency (pointer chasing, cache misses) and risk (OOB access, type confusion). The "intelligence" is external to the data.

## 2. The TSFi Thunk Evolution

We propose replacing passive BDA access with **Active Thunk Attachment**. Instead of just mapping a pointer, we attach a **JIT-compiled Waveform (Thunk)** that "knows" how to traverse and manipulate the data optimally for the current hardware state.

### The Shift:
*   **Old (BDA):** `void* ptr = map_memory(); process(ptr);`
*   **New (Wave-Thunk):** `Thunk* t = attach_waveform(memory_handle); t->execute();`

In this model, the "pointer" is replaced by a **Function Pointer** (the Thunk) that encapsulates:
1.  **Access Pattern:** The optimal stride/tiling for the specific data layout (e.g., Z-order curve vs. Linear).
2.  **Safety:** Inlined bounds checking (like our Segment Checksums) baked into the machine code.
3.  **Operation:** The specific compute kernel (e.g., "Sum", "Rasterize", "Evolve") fused with the load/store logic.

## 3. Waveform Attachment Level

This functionality operates at the "Attachment Level"—the moment memory is brought into the context.

### Mechanism:
1.  **The Event:** A request is made to access a large dataset (e.g., `wave512.attach` or `lau_malloc_gpu`).
2.  **The JIT Decision:** Instead of returning a raw `void*`, the system's **Decision Engine** analyzes the request:
    *   *Is this for sequential read?* -> Generate a pre-fetching linear loader.
    *   *Is this for random access?* -> Generate a hash-indexed loader.
    *   *Is this for mutation?* -> Generate a "Re-entrant" thunk that supports `WAVE(ANY)` splitting.
3.  **The Artifact:** The system generates a small machine code stub (the Thunk) in the `lau_thunk` pool. This stub uses register-relative addressing (`R15`-based in our system) to access the BDA.

## 4. Architectural Benefits

*   **Zero-Cost Abstraction:** The complex logic of `WAVE(ANY)` (splitting, hashing, checking) is compiled *once* when the memory is attached. Subsequent accesses are just a jump to the optimized thunk.
*   **Hardware Alignment:** The JIT can emit AVX-512 instructions if the CPU supports it, or fallback to scalar if not, *transparently* to the calling logic. The BDA "adapts" to the host.
*   **Security:** The Thunk acts as a "Gatekeeper". The raw BDA pointer is never exposed to the higher-level logic (LLM/CLI). Only the trusted, generated Thunk can touch the raw bytes.

## 5. Implementation Strategy

We will leverage the existing `src/lau_thunk.c` and `src/tsfi_zmm_vm.c` infrastructure.

1.  **Extend `ThunkProxy`:** Add a `ThunkProxy_attach_bda(size_t size, uint32_t flags)` API.
2.  **JIT Templates:** Create templates for common BDA patterns (Linear Scan, Stride-Access, Indirect Lookup).
3.  **Integration:** Modify `lau_malloc_gpu` (ReBAR) to return a `ThunkProxy*` handle instead of a raw pointer in strict mode.

This transforms the BDA from a passive "Field" into an active "Fabric" of computation.
