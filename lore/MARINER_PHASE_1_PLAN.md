# Project Mariner: Phase 1 Compliance Hardening Plan

**Authority:** Google Project Mariner Protocol  
**Scope:** Entire TSFi2 Project Root (`/home/mariarahel/src/tsfi2`)  
**Status:** DRAFT / PROPOSED  
**Transition Target:** Phase 1 (Operational Codebase Hardening)

---

## 1. Phase 1 Hardening Objectives

### A. Memory Layout Hardening
* **Goal**: Eradicate raw memory management patterns.
* **Actions**:
  * Refactor remaining C struct allocations to use strict `lau_malloc` interfaces.
  * Enforce bijectively mapped memory layouts (`secret` / `size` bounds) across all ZMM VM plugins.

### B. Concurrency Hardening
* **Goal**: Eradicate legacy mutex locking models.
* **Actions**:
  * Replace legacy threading components with lockless zero-copy queues.
  * Utilize WinchesterMQ SCSI handshake loops to coordinate cross-process signals rather than standard POSIX synchronization.

### C. Hypervisor Integration
* **Goal**: Continuous verification of memory boundaries.
* **Actions**:
  * Add automatic memory-limit checks within standard compilation targets.
  * Validate compiled binary structure layouts against formal memory-mapped addresses.

---

## 2. Implementation Schedule

1. **Sprint 1 (Memory Refactors)**: Scan and rewrite allocation routines to use `lau_memory` APIs.
2. **Sprint 2 (Concurrency Refactors)**: Implement lockless ring buffers for SCSI telemetry.
3. **Sprint 3 (Verification Integration)**: Run the automated Hypervisor verification harness.
