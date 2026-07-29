# Lore: No TSO, No MMU — Direct Register Addressability

The design bottleneck of legacy mainframe systems lay in the dual fallacies of virtual memory page mapping (MMU) and foreground terminal prioritization layers (TSO). By treating the Application Binary Interface (ABI) as an artificial device accessor provider, virtual operating systems introduced significant latency.

## Bypassing the Abstractions

We have proved that WinchesterMQ (`wmq`) renders these layers entirely redundant:

1. **Direct Addressability**: Communication is handled via raw SCSI register handshakes directly on the hardware loopback interface rather than mapping buffers through virtual page tables.
2. **Zero Spooler Latency**: Command queues are executed directly as scheduler tasks, dropping CPU context swapping overhead and bypassing the legacy TSO spooler loop entirely.
3. **Utility Obsolescence**: Standard utility systems like the JCL `IE` helper programs are rendered obsolete by direct WinchesterMQ buffer read/write validations, exposing raw physical device access directly to host interfaces.
