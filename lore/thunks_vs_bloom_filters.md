# Thunks vs. Bloom Filters in TSFi2 Architecture

In the TSFi2 architecture, thunks (specifically `ThunkProxy`) supersede Bloom filters by providing deterministic execution and hardware-aligned performance rather than probabilistic set membership.

### 1. Determinism vs. Probability
Bloom filters are inherently probabilistic, allowing for false positives which introduce "variadic stutter" or entropy into the system. Under the **Axiom 5 Upgrade**, the system mandates **Unified Dielectric Integrity**, requiring bijective (1:1) mappings between state and execution. Thunks provide an absolute resolution of logic; if a `ThunkProxy` is emitted, the path to the underlying `MappedCommon` or hardware routine is guaranteed and verified via provenance keys.

### 2. Zero-Copy Execution (K0Rn Architecture)
A Bloom filter is a passive lookup structure that requires a computational tax (hashing and bit-checking) before data can be accessed. Thunks are active instruction streams. In the **Zhong (ReBAR)** implementation, thunks allow for R15-relative context switching, enabling the CPU to jump directly into GPU-visible memory logic without intermediate lookups. This is the primary driver behind the 26.70 GB/s write bandwidth; the thunk *is* the pointer and the protocol simultaneously.

### 3. Encapsulated Provenance
While a Bloom filter can tell you if a key is "likely" in a set, it cannot carry metadata. TSFi2 thunks encapsulate `LauMetadata`, including allocation source, "secret" offsets, and the **Banach-to-Hilbert (Mu -> YANG -> Eta)** genetic pipeline state. The thunk acts as a "Wired" gateway that enforces safety chains and epoch-synchronization (`c_safety_chain_thunk`) at the instruction level, rather than as an external membership check.

### 4. Hardware Integration
Thunks in `src/lau_thunk.c` utilize JIT-emitted machine code tailored for **AVX-512** register bank compatibility and strict 512-byte alignment. Bloom filters are limited by cache-line latency and bit-masking operations. Thunks bypass the standard branch predictor by providing a direct thunk-to-logic jump, making them mathematically superior for the high-impedance simulation requirements of Dysnomia.
