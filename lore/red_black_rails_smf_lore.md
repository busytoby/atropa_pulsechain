# Red-Black Rails: SMF and the DAT Boundary

In the dual-rail resource layout of the **Auncient** Bendix G-15D / NCR 304 scheduling processor, execution safety is enforced by physical separation of concern:

## The Dual-Rail Split
1. **The Red Rail (Active Scheduling)**: Governs dynamic task dispatching, where job entry threads execute temporary instructions.
2. **The Black Rail (Stable Resource Table)**: Keeps the immutable Program Reference Table (PRT) descriptors, defining persistent memory boundaries and system attributes.

## Wiring SMF to the DAT
All transaction telemetry crossing the boundary between the active Red Rail and the stable Black Rail is recorded sequentially by the System Management Facilities (SMF). 

Every dynamic task transition on the Red Rail triggers an SMF event packet. The SMF engine formats these events into binary record segments and writes them directly into the system database slice (`zmm_system_state.dat.bin`). By mapping this audit trail directly onto the Black Rail's storage registers, the VM ensures that transient runtime operations are permanently recorded in the system's core memory layout.
