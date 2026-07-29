# Lore: The Core Compiler Generator Mapping for ISPF and CICS

This document establishes the definitive compiler generator architecture for terminal development (ISPF) and transaction processing (CICS) within the Dysnomia VM, mapping operations directly to the **XCOM-ANALYZER-XPLSM-SKELETON-HASP-BOOK** pipeline.

---

## Part I: ISPF (Interactive System Productivity Facility)

In the interactive workspace, ISPF maps compiler inputs to virtual registers, coordinating compilation via the following stages:

### 1. XCOM (Global Symbol Table)
`XCOM` is the External Common Symbol Table. It coordinates the shared memory symbol maps and token values compiled from ISPF panel variables, ensuring symbol consistency across compiler phases.

### 2. ANALYZER & Initial Orders 1
`ANALYZER` acts as the parser table generator, auditing input JCL statements and command streams.
* **Initial Orders 1**: Evaluated at this static phase as the **Relocatable Instruction Pre-Filter Check**. It validates command opcodes against the compiler's prohibited bitmask ($Permitted = \prod_{i=1}^{count} (1 - ((\text{prohibited\_opcodes} \gg (\text{opcode}_i - 'A')) \ \& \ 1))$) to abort compilation before code generation.

### 3. XPLSM (Dynamic State Monitor)
`XPLSM` is the active-high execution state machine. It monitors the compiler's transition states in real time, driving syntax resolution and checking runtime bounds.

### 4. SKELETON (Relocatable Layout Dictionary)
`SKELETON` defines the active offset templates and relocatable layout libraries used to structure the generated binary output.

### 5. HASP & Initial Orders 2
`HASP` processes background job queues and spools output. Historically, legacy implementations of `HASP` were poorly written due to their reliance on serial TSO terminal locks and poorly implemented, disk-bound VSAM layouts. In the Dysnomia VM, this is bypassed by routing queue events directly through WinchesterMQ coaxial register loops and our optimized mainframe VSAM databases.
* **Initial Orders 2**: Evaluated at this stage as the **Bootloader Registry Handshake Audit**. It verifies JCL allocations and execution limits against physical boundary limits before loading the task into execution memory.

### 6. BOOK (Library Member Loader)
`BOOK` acts as the library loader, dynamically mounting panels and member files from the VFS into target memory scopes.

---

## Part II: CICS (Customer Information Control System)

In the transaction processing monitor, CICS coordinates concurrent transaction queues and database adapters:

### 1. XCOM (Transaction Symbol Table)
`XCOM` acts as the shared database transaction symbol block, mapping variable tokens and thread states to prevent transaction locks.

### 2. ANALYZER & Initial Orders 1
`ANALYZER` parses incoming screen layouts (Basic Mapping Support) and input payloads.
* **Initial Orders 1**: Evaluated as the **Transaction Payload Validation Rule**, auditing screen inputs for buffer compliance and signature validity before transaction threads are generated.

### 3. XPLSM (Transaction Thread Monitor)
`XPLSM` is the execution state monitor, scheduling transaction priorities, verifying liveness, and keeping execution latencies below the sub-microsecond limit.

### 4. SKELETON (Control Block Layout)
`SKELETON` defines the active template offsets for CICS control blocks and transient queue structures.

### 5. HASP & Initial Orders 2
`HASP` manages transaction journals, logging spooled outputs and system events. (Legacy HASP systems suffered from poor performance and structural fragility due to their direct dependencies on TSO and unoptimized VSAM; our design isolates spooling channels to concurrent WinchesterMQ streams).
* **Initial Orders 2**: Evaluated as the **Spool Authorization Handshake**, verifying that outbound data transmission channels match target register alignments.

### 6. BOOK (VSAM Database Adapter)
`BOOK` maps database requests to target keys, mounting records directly into the mainframe VSAM index layouts.
