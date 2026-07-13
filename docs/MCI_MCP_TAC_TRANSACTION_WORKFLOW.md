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
