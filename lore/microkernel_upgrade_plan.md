# Yul Microkernel Upgrade Path & Lifecycle Plan

To support complex requirements as they arise, our **Shared-State Microkernel Architecture** decouples business logic from state storage. This plan details the mechanics for dynamic logic hot-swapping, state preservation, and security expansions.

---

## 1. Modular Upgradability Framework

The microkernel routes incoming function calls based on their 4-byte selectors. Because it uses `delegatecall`, all state variables (registers, DOS sectors, files, clock height) are preserved in the **Kernel's own storage slots**, completely separating logic from state.

```mermaid
graph LR
    User([User Call]) --> Kernel[Microkernel storage]
    Kernel -- delegatecall --x v1[Driver v1: musicMaker]
    Kernel -- delegatecall -. setDriver .-> v2[Driver v2: musicMaker PRO]
    
    style v1 stroke:#f66,stroke-width:2px,stroke-dasharray: 5 5
    style v2 stroke:#6f6,stroke-width:2px
```

---

## 2. Upgrading a Subsystem (Step-by-Step)

When a driver needs to be upgraded (e.g., supporting high-fidelity voice envelopes in `musicMaker` or nested subdirectories in `diskSystem`):

1. **Compile & Deploy New Logic:**
   Compile the updated strict-assembly Yul contract and deploy it via the singular `ImmutableFactory` (`0x5FbDB2315678afecb367f032d93F642f64180aa3`) to derive its deterministic `CREATE2` address.
2. **Register selectors on Kernel:**
   The owner of the Microkernel calls `setDriver(selector, newDriverAddress)` for the updated function signatures.
3. **Instant Hot-swap:**
   Subsequent user calls to those selectors are immediately routed to the new contract logic. Because the state slots (`0x00`-`0xFF`) remain in the Microkernel contract, the upgrade is completed with **zero data migration** and **zero system downtime**.

---

## 3. Future Enhancements & Extensibility Path

As the operating system grows, we will introduce several architectural components to meet complex requirements:

### A. Access Control Matrices (Slot Guarding)
* **Goal:** Prevent unauthorized user applications from directly overwriting critical registers.
* **Mechanism:** Add a middleware validation layer inside the Kernel fallback routine that maps active sender permissions against target storage slot write ranges before dispatching `delegatecall`.

### B. Dynamic Interface Inspection (ERC-165)
* **Goal:** Allow applications to discover registered driver selectors dynamically.
* **Mechanism:** Register an interface query driver (`supportsInterface(bytes4)`) to query the Kernel's selector routing map.

### C. Driver Chaining (Pipelining)
* **Goal:** Enable middleware interceptors (e.g., event logs, transaction validators).
* **Mechanism:** Allow a selector route to point to a sequence of driver addresses, executing each in turn until the final execution target is reached.
