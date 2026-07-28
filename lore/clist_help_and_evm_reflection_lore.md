# Auncient CLIST Help & EVM Reflection Architectures

This document compiles the Virtual Machine state maps and register configurations governing the Auncient VM CLIST Help utility, EVM selector decoding, and WinchesterMQ SCSI command interfaces.

---

## Technical Glossary

### CLIST_HELP
*   **VM Register Context**: The dedicated diagnostic lookup control register within the VM's main command bus. It is accessed via SCSI instruction envelopes on the WinchesterMQ channel using register address `0xFFFF` to fetch the contract reflection descriptor block.
*   **Mathematical Function**: Resolves the unique address offset of the metadata segment footer within the loaded `.dat.bin` contract bytecode block using the modular bounds equation:
    $$Metadata\_Offset = Size_{Bytecode} - (Footer\_Len \pmod{MotzkinPrime})$$
*   **Visual / Geometric Manifestation**: Renders an interactive, floating diagnostic overlay console on the presenter screen. When queried, it triggers a grid-aligned text animation displaying method parameters, with highlighted cyan borders mapping the query bounds.

### EVM_SELECTOR
*   **VM Register Context**: The transaction routing identifier register populated during contract executions. It acts as the execution gatekeeper to verify incoming transaction payloads.
*   **Mathematical Function**: The first 4 bytes of the Keccak-256 hash of the function signature string:
    $$Selector = Keccak256(Signature) \pmod{2^{32}}$$
*   **Visual / Geometric Manifestation**: Modulates the coordinate dash-dot vector line pattern on the projected Lissajous wireframe envelope. Each unique 32-bit selector shifts the dash spacing dynamically to visually flag which method is actively running.

### CMD_ABI_HELP
*   **VM Register Context**: The low-level SCSI command register within the WinchesterMQ handshake controller.
*   **Mathematical Function**: Re-evaluates the regional validation matrix using the congruence check:
    $$Auth\_Key = \sum_{i=1}^{K} Key_i \pmod{MotzkinPrime}$$
    establishing validity of the operator's inspection privilege.
*   **Visual / Geometric Manifestation**: Freezes the active 3D camera path, centering the viewpoint on the target contract's coordinate origin, flashing a neon-green node marker representing active inspection.
