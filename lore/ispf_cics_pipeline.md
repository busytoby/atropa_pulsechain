# Lore: The Architectural Blueprint of ISPF and CICS within the Dysnomia VM

To achieve sub-microsecond transaction latency and eliminate the synchronous constraints of legacy sharing overlays, the Dysnomia Virtual Machine routes terminal development (ISPF) and transaction processing (CICS) through the concurrent **XCOM-ANALYZER-XPLSM-SKELETON-HASP-BOOK** pipeline. 

This document details the low-level virtual hardware roles, register handshakes, and instruction gating configurations that drive these systems.

---

## Part I: ISPF (Interactive System Productivity Facility)

Within the development environment, **ISPF** acts as the user-facing compiler frontend, coordinating panel logic and dataset manipulations over the following pipeline components:

### XCOM (SCSI Message Transport)
`XCOM` coordinates the transmission and replication of system state datasets, transforming active screen buffers and dataset edits into raw SCSI handshake frames. (Keyboard input and raw terminal interrupts are handled directly by ISPF itself). These frames are transmitted via local loopback sockets to synchronize state registers with the virtual hardware representation.

### ANALYZER & Initial Orders 1
`ANALYZER` evaluates variable strings, panel macros, and JCL variables.
* **Initial Orders 1**: Defined at this compile-time phase as the **Relocatable Instruction Pre-Filter Check**. It audits input commands against a prohibited instruction bitmask ($Permitted = \prod_{i=1}^{count} (1 - ((\text{prohibited\_opcodes} \gg (\text{opcode}_i - 'A')) \ \& \ 1))$). Any mismatch immediately halts template compilation before variables are propagated.

### XPLSM (Liveness Heartbeat Supervisor)
`XPLSM` monitors terminal connectivity. It polls terminal frame coordinates and window visibility states, logging active session heartbeats to guarantee that cursor tracking and overlay scales synchronize with the active compositor cache.

### SKELETON (JCL Template Compiler)
`SKELETON` processes raw templates. Upon successful clearance of `Initial Orders 1`, it expands JCL parameters into 6-bit Fieldata word blocks, preparing job steps without introducing disk latency.

### HASP & Initial Orders 2
`HASP` receives the expanded JCL job streams.
* **Initial Orders 2**: Defined at this runtime phase as the **Bootloader Registry Handshake Audit**. It verifies the compiled JCL steps and target address locations against physical spool limits. If the execution bounds exceed the 48-batch limit, the handshake is aborted, resetting the register configuration.

### BOOK (Panel and Member Loader)
`BOOK` acts as the library loader. It dynamically queries dataset directories from the mainframe VSAM index and mounts active panels and member sources directly into the VFS cache.

---

## Part II: CICS (Customer Information Control System)

Within the transaction processing monitor, **CICS** acts as the high-throughput execution environment, driving database access and message routing:

### XCOM (Fast Transaction Replication)
`XCOM` replicates transaction payloads, datasets, and message streams, translating CICS states into 32-bit register configurations to bypass slow synthetic terminal drivers and enforce thread-safe execution loops. (Keyboard input parsing and event handling are managed directly by CICS itself).

### ANALYZER & Initial Orders 1
`ANALYZER` parses Basic Mapping Support (BMS) screens and transaction arguments.
* **Initial Orders 1**: Evaluated as the **Transaction Payload Validation Rule**. It audits transaction arguments to intercept buffer-overflow attempts and verify FNV-1a DNA signature matches before the thread is spawned.

### XPLSM (Task Thread Scheduler)
`XPLSM` supervises the concurrent transaction threads. It manages locks, handles scheduling priorities, and monitors thread liveness, enforcing latency guards to keep lookup processes below the 1000 ns gate.

### SKELETON (Control Block Generator)
`SKELETON` dynamically generates control block templates for each active task, mapping transaction parameters to transient storage queues (TSQ) and transient data queues (TDQ).

### HASP & Initial Orders 2
`HASP` manages background transaction spools, message logs, and audit journals.
* **Initial Orders 2**: Evaluated as the **Spool Authorization Handshake**. It audits the destination routing headers of outbound spools, verifying that the transmission channel aligns with the WinchesterMQ register boundaries.

### BOOK (VSAM Database Adapter)
`BOOK` handles database interaction. When CICS requests record reads or writes, `BOOK` maps the keys directly to mainframe VSAM KSDS files, routing queries through WinchesterMQ loopback pathways.
