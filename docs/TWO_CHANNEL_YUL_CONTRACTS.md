# Two-Channel Yul Contracts & Unified Peer Architecture

This document describes the design, implementation, and verification of the **Auncient** 2-Channel (2B+D) state-transition architecture and the integration of Yul contract thunks into the unified peer registry.

---

## 1. The 2-Channel (2B+D) State-Transition Model

The execution and verification pipeline is structured to decouple state data from control signaling:

```
                  [ 2-Channel Verification ]
                             /   \
                            /     \
       [ B-Channels: Data ]         [ D-Channel: Control ]
          /           \                       |
  [ B0: Input ]   [ B1: Output ]     [ Opcode / Traversal ]
```

* **B-Channels (Bearer - Data):** Carry active transaction values, addresses, and balances.
* **D-Channel (Delta - Control):** Carries the Merkle path and Yul instruction opcodes.
* **Verification:** The Bitcoin Script verification loop reconstructs state transitions by executing the algebraic invariants matching the D-channel opcode over the B-channel inputs.

---

## 2. Gas Token Invariants (`GasContract.yul`)

The gas contract is compiled directly from Yul source and enforces the following universal invariants:
* **Gas Consumption:** Every read and write transaction is charged a flat fee of $15$ gas tokens. There is no distinction or exemption for view-only operations.
* **Verification Logic:**
  * `OP_GAS_TRANSFER`: $\text{Balance}(A)_{t+1} = \text{Balance}(A)_t - \text{Amount}$
  * `OP_GAS_BURN`: $\text{Balance}(\text{Caller})_{t+1} = \text{Balance}(\text{Caller})_t - \text{GasFee}$

---

## 3. Unified Peer Registry & Thunk Integration

Contracts are loaded into memory as **thunks** and mapped using a Lookup Table (LUT) under the unified peer registry model:

```
                           [ Peer Registry ]
                             /     |     \
            ________________/      |      \________________
           /                       |                       \
    [ PEER_TYPE_MCP ]      [ PEER_TYPE_THUNK ]    [ PEER_TYPE_REGISTER ]
           |                       |                       |
     Remote Server            ALSA Audio           Dynamic Contracts
   (peer_mcp://...)       (peer_thunk://...)     (peer_contract://...)
```

* **Dynamic Resolution:** Address-based resolution (`dynamic_<address>`) is mapped to the target thunk handle.
* **Local Invocation:** Invoking a contract resolves the LUT address to the thunk handle, executing it directly in-memory as a local function to bypass transport overhead.
