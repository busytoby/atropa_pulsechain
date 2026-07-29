# RAU and the RED BLACK DAT Architecture

In the dual-rail resource layout of the **Auncient** Bendix G-15D / NCR 304 scheduling processor, execution safety is managed by the Register Allocation Unit (RAU) over the system state DAT schema (`zmm_system_state.dat.bin`).

## Structural Division of Rails
1. **The Red Rail (Active Scheduling)**: Governs dynamic task dispatching, where job entry threads execute temporary instructions.
2. **The Black Rail (Stable Resource Table)**: Keeps the Program Reference Table (PRT) descriptors, defining persistent memory boundaries and system attributes.

## The Role of the RAU
The stateless coaxial RAU serves as the hardware-level crossbar allocator for CPU registers:
- **Task Isolation**: When a lazy queue worker is spawned, the RAU instantiates a private coaxial page window (such as page `0x5000`) for the worker.
- **Register Gating**: It maps standard task execution registers (such as the Program Counter `PC` and Stack Pointer `SP`) to the active Red Rail slot during processing.
- **Commit Phase**: On step termination, the RAU gates these state parameters back to the stable Black Rail's descriptors in the DAT.

By auditing these transition coordinates via SMF logs, the system maintains a secure, crash-resilient record of all queue operations.
