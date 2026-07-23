# Auncient Network Device Stack Architecture

This document describes the design, implementation, and routing methodologies of the network device stacks bridging WinchesterMQ SCSI, DECnet, and STANAG 5516 protocols using XPL on the ZMM VM.

---

## 1. Protocol Stack Topography

The network interface is modeled as a standard four-layer system architecture:

```
+-------------------------------------------------------------+
| PRESENTATION: STANAG 5516 Tactical Frames (.dat.bin)        |
+-------------------------------------------------------------+
| TRANSPORT: DECnet Phase IV Node Routing (0xF600 Shared SRAM) |
+-------------------------------------------------------------+
| LINK LAYER: Local Device Command Queue (0xF900 Cache)       |
+-------------------------------------------------------------+
| PHYSICAL: WinchesterMQ SCSI Phase Loops (0xFB00 Status)    |
+-------------------------------------------------------------+
```

---

## 2. Layer Integrations and Interfaces

### A. Physical Layer (SCSI / WinchesterMQ)
* **Status Register**: Mapped to absolute memory address `0xFB00` (`64256` bytes).
* **Execution**: Tracks WinchesterMQ handshake phase loops (REQ/ACK). Setting `0xFB00` to `2` signals a **Data Out** phase, indicating a packet has been routed to shared memory and is ready for hardware transmission.

### B. Link Layer (Local Device Stack)
* **Stack Base**: Mapped to absolute memory address `0xF900` (`63744` bytes).
* **Execution**: Acts as a private, high-speed L1 cache buffer storing raw data frames prior to network routing.

### C. Transport Layer (DECnet Node Routing)
* **Shared Space**: Mapped to absolute memory address `0xF600` (`63000` bytes).
* **Execution**: The routing engine parses the destination node from the incoming frame. 
  * If the destination is the `LOCAL_NODE_ID` (`5`), the packet payload is copied directly to the Local Stack (`0xF900`).
  * If the destination is an external node (e.g., `BRIDGE_NODE_ID` `12`), the packet is written to the Shared Stack (`0xF600`) and the SCSI status register is set to `2` to trigger outward hardware routing.

### D. Presentation Layer (STANAG 5516)
* **Encapsulation**: Serializes data packages into STANAG 5516 Link-16 tactical frame formats:
  * **Magic Header (2B)**: `0x5516` (identifies valid Link-16 frame).
  * **Source Node (1B)**: Sender address.
  * **Destination Node (1B)**: Recipient address.
  * **Payload Data (16B)**: Raw telemetry array.
  * **Frame Checksum (1B)**: Evaluates sum of payload bytes to verify transmission integrity.

---

## 3. Reference Implementations

* **XPL Library**: See [xpl_network.xpl](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/xpl/xpl_network.xpl).
* **Verification Tests**: See [test_xpl_network_lib.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_xpl_network_lib.c) and [test_xpl_network_stack.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_xpl_network_stack.c).
