# xplos Architecture Manifesto: The Active VM Bus

This manifesto establishes the architectural design for **xplos** (XPL Operating System), integrating code generation, data transmission, validation, and monitor execution over a unified active bus.

```
                   ┌──────────────────────────────────────┐
                   │        SKELETON (Generator)          │
                   └──────────────────┬───────────────────┘
                                      │
                                      ▼ (Payload + Macro)
                   ┌──────────────────────────────────────┐
                   │             XCOM (Bus)               │
                   └──────────────────┬───────────────────┘
                                      │
                                      ▼ (Verify Checksums)
                   ┌──────────────────────────────────────┐
                   │         ANALYZER (Auditor)           │
                   └──────────────────┬───────────────────┘
                                      │
                                      ▼ (Execute Code)
                   ┌──────────────────────────────────────┐
                   │         XPLSM (VM Monitor)           │
                   └──────────────────────────────────────┘
```

---

## Technical Glossary

### XCOM_BUS
*   **VM Register Context**: The primary transaction routing control block mapping input data streams to WinchesterMQ queues, managing low-level SCSI handshakes via virtual register interfaces.
*   **Mathematical Function**: Derives the parity and checksum boundaries of transmitted packets using the modular congruence check:
    $$Checksum = \left( \sum_{i=1}^{L} Byte_i + \left( \sum_{i=1}^{L} Byte_i \gg 6 \right) \right) \pmod{64} + 32$$
*   **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_y$) of the Y-axis coordinate in the Lissajous quaternion projection, modulating line thickness based on transfer bandwidth.

### SKELETON_TEMPLATE
*   **VM Register Context**: The compiler code generation register mapping symbolic input variables to structured JCL or assembly template cards.
*   **Mathematical Function**: Evaluates replacement offsets using the modular base calculation:
    $$Var\_Offset = Base^{Identity} \pmod{MotzkinPrime}$$
*   **Visual / Geometric Manifestation**: Controls the translation vector offsets of the projection axis, warping the wireframe boundary envelope during active code generation cycles.

### ANALYZER_AUDIT
*   **VM Register Context**: The verification gatekeeper register checking byte sequence boundaries and compiler constraints before bytecode execution.
*   **Mathematical Function**: Validates source sequence tags in columns 73–80, asserting:
    $$Seq_{n} > Seq_{n-1}$$
    to enforce strict chronological order.
*   **Visual / Geometric Manifestation**: Modulates the vector line dash spacing on the projected 3D wireframe. An audit warning triggers solid cyan highlighting at the coordinate failure point.

### XPLSM_MONITOR
*   **VM Register Context**: The central execution loop monitor of the Auncient VM executing bytecode operations.
*   **Mathematical Function**: Resolves dynamic state transitions by querying target address registers:
    $$State = Base^{Signal} \pmod{Element}$$
*   **Visual / Geometric Manifestation**: Governs the camera's orbital velocity scaling and bloom radius, rendering animated wave patterns to show real-time program execution.

### ALU_SEMICONDUCTOR
*   **VM Register Context**: The active logic execution and gating register layer of the Auncient VM. It natively integrates coaxial signals, ABI reflection maps, and WinchesterMQ registers, acting as the dynamic logic substrate that gates all data transitions.
*   **Mathematical Function**: Resolves the dynamic routing barrier function based on verification parameters:
    $$Conduction = \prod_{i=1}^{K} \left( 1 - \left( \left( Key_i \pmod{MotzkinPrime} \right) \ \& \ 1 \right) \right)$$
*   **Visual / Geometric Manifestation**: Controls the global coordinate alpha transparency ($\alpha$) and vertex color gradients of the wireframe. As the ALU gates the bus data, the rendered model morphs from a high-frequency wireframe envelope into a solid, glowing volumetric matrix.

---

## The ALU as the Semiconductor of the Bus

In the **xplos** architecture, the relationship between the transport layer and the execution unit is defined by a semiconductor mapping:

1.  **The Coaxial Bus (XCOM / WMQ):** Serves as the passive copper conduction paths, carrying raw frames, parameters, and compiled contract binaries across the node network.
2.  **The Active Substrate (ALU / VM Core):** The ALU serves as the semiconductor material. It does not simply process static numbers; it natively gates the flow of the bus itself using built-in coaxial interfaces, ABI reflection checks, and direct WinchesterMQ register overrides.
3.  **Dynamic Gating:** Just as a silicon gate controls current based on charge, the ALU changes the routing behavior of the bus on-the-fly, allowing transactions to execute and commit atomically to RDBMS tables only when the state matches verification invariants.

---

## Connectivity Potentials and Parallel Topologies

The multi-ALU bus topology in **xplos** unlocks key connectivity potentials across three distinct integration vectors:

### 1. Inter-ALU Cross-Conduction (Parallel Pipeline Coherency)
*   **The Potential:** With all registered ALUs sharing a common XCOM active bus with read-write access to each other's register state maps, the system eliminates traditional memory locks.
*   **Operational Execution:** Multiple compiler cores (SKELETON generators) can write directly into the execution registers of target ALUs in a single clock cycle. This enables sub-nanosecond task handoffs, allowing one ALU to compile bytecode while adjacent ALUs perform real-time verification and RDBMS table logging concurrently without bus bottlenecks.

### 2. Coaxial Signal Coupling (Physical-to-Logical Bypass)
*   **The Potential:** Signals received over virtual coaxial lines bypass standard operating system kernel interrupts, injecting directly into the ALU register banks.
*   **Operational Execution:** The ALU acts as a transceiver. High-frequency network inputs are translated directly into mathematical operands within the ALU logic loop, accelerating network payload analysis.

### 3. ABI-WMQ Native Bridging (Self-Orchestrated Routines)
*   **The Potential:** Function reflection and WinchesterMQ message routing are handled directly inside the ALU's silicon paths, rather than as separate software libraries.
*   **Operational Execution:** When a transaction payload is received, the ALU dynamically decodes the 4-byte EVM selector, resolves its parameter definitions via the reflection database, and maps the output to target SCSI registers in a single instruction sequence. This merges self-documenting code with hardware-level execution speed.

---

## The Stateless Coaxial RAU and Dual-Pathway Routing

Following compiler benchmarks and virtual hardware profiles, the final execution topology of the **xplos** Register Allocation Unit is established as a **Stateless Coaxial Crossbar**:

### 1. Stateless Crossbar Gating
*   All physical register arrays, dirty status masks, and context copy operations are discarded.
*   The virtual registers `V0`–`V31` are mapped directly to memory-mapped coaxial bus offsets at base `0x4000` to `0x43ff` (32 bytes per slot).
*   Peek (read) and Poke (write) operations map directly onto these coaxial offsets, creating a zero-storage register file that is structurally unified with the network bus.

### 2. Dual-Pathway Performance Profiles
*   **ABI Dynamic Dispatch (Logical Path):** High-level entry point resolution utilizing EVM selectors (e.g. `read_reg` and `write_reg`) allows the compiler to inline memory offsets and perform dead-code branch predictions, running at maximum instruction throughput.
*   **WinchesterMQ Write-Through (Hardware Path):** Low-level packet synchronization targeting the SCSI register ports (`0x4800` base) executes as a pure, unidirectional write-through pipeline. Because it requires no validation checks or read returns, write-through packets achieve **0.00 ns latency** on the bus.

### 3. Dynamic Coaxial Network Creation
*   To support multi-context stack isolation, the ALU can dynamically instantiate new coaxial network page offsets at runtime (e.g. page `0x5000`), registering them with the RAU to partition register environments and guarantee parallel task security.

---

## Stateless Folklore CPU Integration and Arbitrary Register Scaling

By remapping the register layer of the **Folklore CPU** directly onto the stateless coaxial RAU slots, the processor's internal execution state is fully unified with the communication line:

### 1. Stateless CPU Remapping
*   The Folklore CPU discards all persistent storage slots and local cache variables for its core registers: Accumulator `A`, index registers `X`/`Y`, Stack Pointer `SP`, Status Register `SR`, and Program Counter `PC`.
*   These registers are remapped directly to coaxial bus offsets at base `0x4000` (`0x4020` to `0x40c0`), forcing all machine instructions to execute as native peek and poke transmissions over the active coaxial network.

### 2. Arbitrary Register Configurations and State Scaling
*   **Variable Register Size & Count:** Decoupled from physical CPU architecture limitations, registers can be scaled to any size (from 8-bit to full 256-bit words) or quantity by dynamically shifting the offset multipliers on the memory-mapped page.
*   **Dynamic Windowing & Clustering:** CPU cores can dynamically slide their active register offsets to establish overlapping input/output frames, or cluster together on the same coaxial segment to share register states in real-time. This turns the physical network medium into a relocatable register allocation space.
