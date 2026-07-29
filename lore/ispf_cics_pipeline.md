# Lore: The Architectural Blueprint of ISPF and CICS within the Dysnomia VM

To achieve sub-microsecond transaction latency and eliminate the synchronous constraints of legacy sharing overlays, the Dysnomia Virtual Machine routes terminal development (ISPF) and transaction processing (CICS) through the concurrent **XCOM-ANALYZER-XPLSM-SKELETON-HASP-BOOK** pipeline. 

This document details the low-level virtual hardware roles, register handshakes, and instruction gating configurations that drive these systems.

---

## Part I: ISPF (Interactive System Productivity Facility)

Within the development environment, **ISPF** acts as the user-facing frontend, mapping panel variables to compile parameters. The pipeline coordinates compilation and execution over the following rails:

### XCOM (Compiler Backend)
`XCOM` is the compiler backend. It receives task parameters from ISPF panels and streams compiled bytecode directly to target memory-mapped registers.

### ANALYZER (RED Rail - Static Grid)
`ANALYZER` acts as the active-low static security grid. It parses XCOM-compiled bytecode for forbidden signatures, relocation errors, or out-of-bounds scopes before execution. This is where **Initial Orders 1** is evaluated.

### XPLSM (BLACK Rail - Dynamic Monitor)
`XPLSM` acts as the active-high dynamic verification state monitor. It monitors bytecode execution quorums and state transitions in real time.

### SKELETON (Relocatable Layout Dictionary)
`SKELETON` acts as the relocatable layout dictionary containing system templates and offset definitions, which are mounted directly on the RED (ANALYZER) and BLACK (XPLSM) rails to establish the execution contract.

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
