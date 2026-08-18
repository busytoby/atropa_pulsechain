# Modular LLM Binary Token Specification (`.dat.bin`)

## 1. Overview

Each Large Language Model (LLM) and specialized domain prover in the TSFi2 CP/M-Tomie ecosystem operates as an independent, self-contained **binary token file** ending in `.dat.bin`.

This design conforms to Rule 13 (pure binary quadtree layout) and provides PC compatibility, zero-copy in-memory execution, and immediate mountability across recursive sub-virtual machines.

```
       ┌───────────────────────────────────────────────────────────┐
       │     STANDALONE LLM BINARY TOKEN (.dat.bin) LAYOUT         │
       ├───────────────────────────────────────────────────────────┤
       │ [0x00000000 - 0x00000007]: 8-Byte System Header           │
       │  • 0x57A10000 | (MotzkinPrime & 0xFFFF)                   │
       ├───────────────────────────────────────────────────────────┤
       │ [0x00000008 - 0x0003E807]: 256KB Vocabulary & Traits     │
       │  • 32,000 Embedded Tokens (8 bytes per trait entry)       │
       │  • SMPL Pose Vectors & Latent Projection Attributes       │
       ├───────────────────────────────────────────────────────────┤
       │ [0x0003E808 - 0x0003E80B]: 4-Byte Descriptor Count        │
       │  • uint32_t entry_count                                   │
       ├───────────────────────────────────────────────────────────┤
       │ [0x0003E80C - End of File]: Formal Prover Executables     │
       │  • N Sequential 90-Byte Binary Descriptors                │
       │  • CP/M Transient Program Area (TPA 0100H) Headers        │
       │  • Certified Rule 18 3-Term Recurrence Checksums          │
       └───────────────────────────────────────────────────────────┘
```

---

## 2. Binary Layout Specification

### 2.1. System Header (8 Bytes)
* **Magic Signature**: `0x57A1` in the high 16 bits of the lower 32-bit field.
* **Modular Divisor Field**: Low 16 bits contain `MotzkinPrime & 0xFFFF` (`0x1EBB`), asserting field alignment with the Auncient Dysnomia VM.

### 2.2. Vocabulary & Trait Embeddings (256,000 Bytes)
* **Capacity**: 32,000 distinct token traits.
* **Format**: Contiguous 64-bit (`uint64_t`) hash representations generated via deterministic FNV-1a modular folding.

### 2.3. Formal Theorem Prover Corps
* **Count Field**: 32-bit integer indicating the total count of packaged theorem engines.
* **Structure**: Fixed 90-byte descriptors containing FourCC identifier, opcode signature, title, theorem records, and verified Rule 18 checksums.

---

## 3. PC & Virtual Machine Compatibility

* **Little-Endian Standard**: Fully compatible with standard x86/x86_64 host processors and Dysnomia ZMM virtual registers.
* **Transient Program Area Alignment**: All packaged instruction sets target base address `0x0100` (`CPM_TPA_BASE`), permitting immediate in-place execution without runtime relocation.
* **SCSI Loopback Streaming**: Supports raw streaming across WinchesterMQ SCSI channels for child VM deployment.

---

## 4. Coaxial Attached PC Direct Quadtree Streaming

Binary `.BIN` executables running deep within nested CP/M-Tomie virtual machines possess direct, transparent access to `.dat.bin` quadtree assets residing on the coaxially attached PC host:

```
       ┌───────────────────────────────────────────────────────────┐
       │     COAXIAL PC-TO-NESTED-VM STREAMING TOPOLOGY            │
       ├───────────────────────────────────────────────────────────┤
       │ [Host PC Storage Plane]                                   │
       │  • assets/*.dat.bin (269.4KB Quadtree Model Slices)       │
       │  • Direct Memory-Mapped Disk Buffers                      │
       └─────────────────────────────┬─────────────────────────────┘
                                     │
                        (Coaxial Loopback Socket /
                         WinchesterMQ SCSI Handshake)
                                     │
                                     ▼
       ┌───────────────────────────────────────────────────────────┐
       │ [CP/M-Tomie Root Virtual Machine (TPA 0100H)]             │
       │  • Bridges Coaxial Streams via Blue Box Kermit/STANAG     │
       └─────────────────────────────┬─────────────────────────────┘
                                     │
                        (Multi-Tier Recursive Channel)
                                     │
                                     ▼
       ┌───────────────────────────────────────────────────────────┐
       │ [Deep Nested CP/M-Tomie Child VMs: .BIN Executables]      │
       │  • Direct in-place streaming of .dat.bin quadtree slices  │
       │  • Zero-copy token embedding lookups (< 250 us latency)   │
       │  • In-memory Rule 18 checksum parity execution            │
       └───────────────────────────────────────────────────────────┘
```

* **Coaxial Channel Protocol**: The connection operates over raw loopback sockets and WinchesterMQ SCSI handshakes, allowing nested `.BIN` programs to issue sector reads directly against the host PC's `.dat.bin` file descriptors.
* **Transparent Multi-Tier Forwarding**: Child virtual machines at arbitrary nesting depths transparently forward packet requests through parent VM instances to stream quadtrees directly from host storage.
* **Zero Intermediate Buffering**: Streamed `.dat.bin` slices load straight into the child VM's active execution memory without requiring intermediate local disk staging.

---

## 5. Operational Guidelines

1. **Naming Standard**: All model tokens must use filenames ending in `.dat.bin` (e.g., `tsfi2_custom_cpm_tomie.dat.bin`, `cpm_tomie_acoustic.dat.bin`).
2. **In-Memory Invocation**: Engines must load directly via zero-copy memory mapping and execute via native function pointers without creating intermediate `.c` or object files on disk.
3. **Parity Validation**: Every token file must satisfy Rule 18 3-term orthogonal polynomial recurrences prior to execution authorization.
4. **Coaxial Direct Streaming**: Deeply nested `.BIN` binaries wanna stream `.dat.bin` quadtrees straight from the coaxial attached PC across loopback sockets with sub-microsecond latency.
