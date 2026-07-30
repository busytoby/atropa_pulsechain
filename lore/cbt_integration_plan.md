# CBT Tape Integration & Porting Plan

This document outlines the phased process for identifying, classifying, and implementing relevant utilities from the 1,000+ files of the CBT Tape library into our lightweight cooperative environment.

---

## Phase 1: Catalog Indexing & Classification

Before porting utilities, we establish an automated catalog classification system to filter the 1,000+ tape files. Files are categorized into four priorities:

| Priority | Category | Key File Examples | Action |
| :--- | :--- | :--- | :--- |
| **High** | Decompression & VFS Mounting | File #002 (CBT973), File #001 (Index) | Direct porting of decompressors and directory listing accessors. |
| **Medium** | JCL/ISPF Task Orchestration | File #008 (SYSUPLOG procedures) | Parsing control cards, condition steps, and session parameters. |
| **Low** | SMF & Performance Audit | File #015 (Warner Bros), File #019 (John Hooper) | Light auditing utilities (e.g. `cbtsmf` CPU logs). |
| **Bypassed** | Hardware Exits & Tape Drivers | File #006 (Physical tape copy exits) | Excluded. Retain hardware emulation layer instead. |

---

## Phase 2: In-Memory Stream Extraction

To access file contents without file-system pollution, we use the compiled OpenSSL socket client to download targets on-demand:
1. **Dynamic Streaming**: Retrieve target zip files from `cbttape.org` into memory buffers.
2. **PDS Mounts**: Stream uncompressed sequential data records directly into the local VFS.
3. **Partition Mapping**: Format target files using the 256-byte PDS directory layout to allow clean member extraction.

---

## Phase 3: Phase-by-Phase Implementation Pathway

### Phase 3.1: Core Accessors & Catalog Resolvers
* Maintain metadata search utilities (`cbtsearch`, `cbtauth`, `cbtversion`) to query index structures.
* Resolve Generation Data Groups (GDGs) and DCB parameters directly from catalog mapping tables.

### Phase 3.2: JCL & ISPF Parity Orchestration
* Run parsed batch scripts under the cooperative `XplosScheduler`.
* Support parameter overrides, temporary dataset allocations (`&&temp`), and step-level condition checks.
* Enable session variable pools (`VGET`/`VPUT`) for interactive ISPF tasks.

### Phase 3.3: Binary PDS Layout Compatibility
* Enforce MVS-compatible Directory Blocks (256-byte headers) on `.dat.bin` stores.
* Support dynamic member additions and list directory reads to maintain parity with mainframe datasets.

---

## Phase 4: Integration Verification

* **Unit Test Coverage**: Run structural assertions on parsing, math, and variable pool lookup functions.
* **Cooperative Scheduling**: Verify all commands yield control cleanly and maintain sub-microsecond lookup latencies.
