# Lore: The Deprecation of TSO, TSSO, and TSSSO in Favor of the SKELETON-HASP-BOOK Pipeline

Historically, legacy CBT Tapes relied on a series of nested, serial scheduling overlays to coordinate user terminal interactions and job execution streams. These were classified under the TSO (Time Sharing Option), TSSO (Temporary System Spool Session Overlay), and TSSSO (Tri-State Spool Subsystem Overlay) architectures. 

These legacy architectures introduced significant locking overhead and serial latencies due to their reliance on slow, disk-bound queue structures and synchronous virtual terminal interrupts.

To resolve these inefficiencies and establish sub-microsecond transaction latency, the modern Dysnomia VM deprecates the entire TSO/TSSO/TSSSO stack. It replaces it with the concurrent, hardware-loopback aligned **SKELETON-HASP-BOOK** pipeline, which routes execution parameters directly through WinchesterMQ register boundaries.

---

## The Legacy Stack: TSO, TSSO, and TSSSO

### 1. TSO (Time Sharing Option)
Originally designed to manage interactive user terminal sessions, TSO scheduled execution slices serially. Under high hardware loads, TSO locked the main supervisor loops, causing screen refresh bottlenecks and coordinate scaling dropouts.

### 2. TSSO (Temporary System Spool Session Overlay)
TSSO functioned as a secondary scheduler overlay, catching background spools from active TSO sessions and staging them for batch submission. This intermediate staging step introduced redundant disk writes and serialization points.

### 3. TSSSO (Tri-State Spool Subsystem Overlay)
TSSSO managed the tri-state scheduling logic (Pending, Executing, Completed) across partitioned virtual boundary segments. It operated as a macro-scheduler overlay, verifying JCL jobs against static memory maps before routing them back to the active console terminal, creating a massive execution bottleneck.

---

## The Modern Solution: SKELETON, HASP, and BOOK

The replacement pipeline operates concurrently at the low-level virtual hardware layer, eliminating intermediate scheduling steps and aligning directly with the `DisplacementShader` vertex coordinate boundary constraints.

### Phase 1: SKELETON (JCL Template Expansion)
Rather than executing static, pre-allocated JCL streams, the system utilizes `SKELETON` templates. When an execution request is initialized, the `SKELETON` compiler expands variables dynamically, binding execution parameters to active registry maps without disk I/O.

### Phase 2: HASP (Concurrent Spooling)
The expanded job stream is fed directly to the `HASP` spooler. `HASP` schedules queue execution paths concurrently, utilizing low-level WinchesterMQ SCSI handshake loops to bypass standard drivers and transmit spool logs via register states.

### Phase 3: BOOK (Dynamic Member Extraction)
When `HASP` requires library modules or source datasets to complete a step, the `BOOK` utility intercepts the catalog request. Instead of performing standard file search routines, `BOOK` queries the mainframe VSAM index and mounts PDS members dynamically into the active VFS structure, mapping records directly to WinchesterMQ register addresses.

---

## Architectural Alignment with WinchesterMQ

By utilizing the **SKELETON-HASP-BOOK** pipeline, program execution loops scale in perfect synchronization with the 8-bit wrap register boundaries (`REG_DISP_X`) enforced by the `DisplacementShader`. When a queue event triggers:
1. `SKELETON` formats the memory allocation offsets.
2. `HASP` routes the instruction block via SCSI loops.
3. `BOOK` resolves dataset members through VSAM indexes.

This guarantees sub-microsecond lookup latency and prevents wireframe geometry collapse, verifying compliance standards automatically across all active VM states.
