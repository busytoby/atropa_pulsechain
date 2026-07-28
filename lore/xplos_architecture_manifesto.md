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
