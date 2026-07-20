# Specification: Ephemeral Yul / Local Quadtree Persistence Pipeline

This document defines the interface and lifecycle rules for the execution of Yul contract objects under the custom BTC Script gas token, balancing ephemeral transaction execution with immutable local storage.

---

## 1. The Separation of Concerns

To preserve complete Bitcoin compatibility, the system enforces a strict boundary between execution state and storage:

1.  **BTC Script System (On-Chain/EVM)**:
    *   Acts as the transaction gate.
    *   Handles signature verification, witness calculations, and gas accounting.
    *   Execution is **ephemeral**: Yul contract code is loaded onto the stack dynamically to verify the state transition, but is completely cleared from the active EVM storage context once the transaction commits.
2.  **Local Quadtree Database (Off-Chain/Disk)**:
    *   Acts as the immutable archive.
    *   Stores compiled contract objects (`.bin`) and ledger state history as binary `.dat.bin` files.
    *   Protects contracts from EVM state manipulation or storage alterations.

---

## 2. Transaction Lifecycle Pipeline

```
  [ Local Disk ]              [ ZMM / Anvil EVM ]
   Quadtree DB                 Transaction Stack
        |                              |
  1. Load Bytecode (.bin) ------> 2. Push to Stack
                                       |
                                  3. Run BTC Script Witness
                                       |
                                  4. Execute Yul Code (Right Branch)
                                       |
  6. Commit Root <--------------  5. Check output (Unify)
                                       |
                                  7. Clear Stack (Discard Code)
```

### Step 1: Bytecode Retrieval
The ZMM hypervisor receives an address-based call query (e.g. targeting `dynamic_<address>`). It parses the target address, opens the corresponding local quadtree `.dat.bin` ledger slice, and reads the immutable contract bytecode.

### Step 2: Ephemeral Stack Loading
The bytecode is packaged into the transaction data payload and pushed onto the execution stack. No permanent EVM `SSTORE` allocation is created for the contract's instruction set.

### Step 3: BTC Script Gas Gating
The Left branch of the active 2-stack execution node verifies the transaction's signature and witness proof, deducting gas from the caller's balance pool.

### Step 4: Yul Execution
The Folklore CPU executes the contract bytecode ephemerally. Any required state mutations must write exclusively to the middle stack unification branch.

### Step 5: State Unification and Committing
The output state on the stack is unified against the transaction constraints. Upon validation success, the new Merkle root is calculated and committed to the local Quadtree block ledger on disk.

### Step 6: Memory Reclamation
The transaction completes. The execution frame is discarded, and the stack memory containing the contract bytecode is overwritten (`memset(..., 0)`), leaving no residual contract trace in the active EVM memory map.
