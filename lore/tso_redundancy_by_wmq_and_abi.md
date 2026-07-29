# Proof of TSO Command Redundancy via WinchesterMQ & System ABI

Traditional mainframe architectures require Time Sharing Option (TSO) command tables to allocate foreground memory buffers, validate execution authorization keys, and map terminal session lines. In the **Auncient** system, this layered overhead is eliminated by the introduction of WinchesterMQ (`wmq`) and the low-level system ABI.

## 1. WinchesterMQ (WMQ) Hardware-Level Dispatch

The WinchesterMQ virtual hardware SCSI handshake loops bypass TSO interactive command processors entirely:
* **Direct Channel Injection**: Instead of translating keyboard scan codes through TSO line buffers, commands and transactions are packed directly into 512-byte SCSI sectors on the `wm` device.
* **Non-Blocking Interrupts**: The WinchesterMQ controller generates hardware-level interrupts when a sector is ready, invoking the scheduler task dispatcher immediately. There is no terminal line pooling or foreground/background priority swapping required.

## 2. ABI-Level Scheduler Integration

The system ABI exposes direct entry points that replace TSO command table lookup layers:
* **Unified Execution Pathway**: Commands are dispatched using `tsfi_xplos_shell_exec()`. This directly registers execution blocks as scheduler tasks rather than looking up symbols in TSO authorized tables (`IKJEFTE2`).
* **State Preservation via Registers**: Terminal session states are tracked via native registers and control structures (`PC`, `SP`) rather than relying on heavy TSO logon profiles. 
* **Zero-Thread Idle Nodes**: When the incoming command queues are empty, worker nodes drop to a 99.9% idle state, yielding CPU processing frames automatically. Under TSO, terminal listener threads must be maintained constantly.

By routing all operator and automated instructions directly through the WinchesterMQ channel into the scheduler task queue, the TSO authorized command structures are rendered fully redundant.
