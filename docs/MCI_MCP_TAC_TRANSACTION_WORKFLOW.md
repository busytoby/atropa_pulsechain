# MCI MCP TAC Transaction Workflow

This document describes the transaction submission workflow using the **Auncient** MCI (Coaxial Infrastructure), MCP (Gateway Server), and TAC (Helmholtz Endpoint) system.

---

## 1. Workflow Architecture

The transaction pipeline maps network inputs to physical in-memory executions:

```
[ TAC Peer ] --(1. Dial-In)--> [ MCP Gateway Server ]
                                          |
                                    (2. Modulate)
                                          v
                              [ MCI Coaxial Bus (LDS) ]
                                          |
                                    (3. Intercept)
                                          v
                              [ TAC Helmholtz Endpoint ]
                                          |
                                     (4. Execute)
                                          v
                                     [ ZMM VM ]
```

---

## 2. Step-by-Step Execution Sequence

### Step 1: Peer Dial-In
* The peer establishes a connection to the MCP Gateway Server.
* The MCP server authenticates the peer connection against the unified registry mapping (`PEER_TYPE_MCP`).

### Step 2: Coaxial Signal Modulation (MCI)
* The MCP gateway formats the transaction parameters into the 2-Channel layout:
  * **B-Channels:** Carry sender/recipient balances and values.
  * **D-Channel:** Carries the Yul instruction opcodes.
* The payload is modulated as frequency coefficients across the virtual coaxial activity bus (MCI).

### Step 3: Zero-Copy Interception by the TAC
* The Helmholtz command-capable endpoints (acting as the TAC) intercept the modulated coaxial signal directly from shared memory (LDS/VGPRs) using zero-copy pointers.
* The TAC performs a pre-flight gas check against `GasContract.yul` to ensure the caller has sufficient balance.

### Step 4: ZMM Execution
* The TAC translates the D-channel opcode into a semantic Helmholtz command.
* The command is written to `ws->current_directive` and processed instantly in-memory by the ZMM execution loop (`ws->step_safety_state()`), updating the state root.

---

## 3. RSTS/E-Style Multi-User Account Hierarchy (PPN & Keys)

To support multiple ACLs per connection, caller identities are partitioned using a Project-Programmer Number (PPN) structure:

```
[ PPN Account: Project, Programmer ]
                 |
        _________________
       /                 \
  [ Key A (PKI) ]   [ Key B (PKI) ]
```

* **PPN Account:** A logical billing/permissions group. A single PPN may authorize multiple unique PKI keys.
* **Key-Based Verification:** The system confirms the public key ID matches one of the active keys registered to the PPN prior to executing the state changes.
* **Default 11 Key:** The default system administrative key (`Key 11`) is pre-authorized for the system manager PPN `[1, 1]`.

---

## 4. Helmholtz-Compatible Keyboard Monitor (KBM) Commands

Standard RSTS/E KBM commands are routed through the Helmholtz semantic translation layer:
* **`SYSTAT`:** Evaluates and writes the status of active card process slots, current execution epochs, and gas balances directly to the virtual system log.
* **`FORCE <card_id> <directive>`:** Allows authorized administrators (PPN `[1, 1]` using `Key 11`) to override the instruction register of a specific card, forcing state transitions (e.g. `HELMHOLTZ_RESONATE`) during the next scheduling cycle.

---

## 5. Dynamic Capability Flags (MCP ACL)

Specific capability privileges are distributed dynamically at the MCP layer, bypassing the need for core VM mutations:
* **`can_bypass_gas`:** Allows execution of transactions with zero gas fees (used for administrative updates or bootstrapping).
* **`can_write_any_storage`:** Grants administrative bypass to modify any account or slot index directly.
* **`can_mount_devices`:** Authorizes mounting or modifying virtual disk device paths (e.g., `DK0:`).

