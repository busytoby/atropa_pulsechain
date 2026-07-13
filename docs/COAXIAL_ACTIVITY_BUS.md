# Auncient Coaxial Activity Bus (ACAB)

This document describes the design, implementation, and verification of the **Auncient Coaxial Activity Bus (ACAB)**, optimized for proving state transition activities on UTXO-based ledgers (like Bitcoin).

## 1. Concept: Proof of Activity vs. Proof of State
Unlike account-based architectures (e.g. EVM) where contract state is stored persistently on-chain, UTXO-based ledgers only record transaction spent transitions (activities). The ACAB maps coordinate-ordered activity events off-chain and registers them on-chain using cryptographic proofs:

* **Activity Commitments:** Off-chain actions (deposits, channel spent flags, register modulations) are assigned to specific index coordinates.
* **Merkle Attestation:** The coordinate-ordered activities are hashed as leaf nodes in a Merkle tree. The 32-byte Merkle root is broadcast on-chain as a proof of activity.
* **On-Chain Verification:** Peer nodes verify target actions by submitting Merkle path proofs (siblings). The Bitcoin scripting engine verifies the path against the published root using standard hashing opcodes (`OP_SHA256` / `OP_EQUAL`).

## 2. Merkle Coordinate Layout
For a 4-channel activity layout:
```
               [Activity Root Hash]
                    /       \
             [Parent 0-1]  [Parent 2-3]
               /      \      /      \
             [L0]    [L1]  [L2]    [L3]
```
Where each leaf $L_i$ hashes the coordinate index and transaction transition details:
$$L_i = \text{SHA256}(\text{Index}_i, \text{Type}_i, \text{Value}_i)$$

## 3. Verification & Tests
* **Prover Engine:** Compiled and verified in [test_auncient_activity_bus.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tests/test_auncient_activity_bus.c).
* **Proof Generation:** Hashes coordinate events (e.g., Index 2: `COAXIAL_MODULATION`, Value `440`), builds the parent levels, and verifies the proof path against the composite Root Hash successfully.

## 4. Balanced 2-3 Tree Integration
To support dynamic, self-balancing activity registers, the ACAB can be compiled as a Merkle 2-3 Tree:
* **Node Degrees:** Internal nodes can have either 2 children (2-node) or 3 children (3-node).
* **Dynamic Hashing:**
  * 2-node: $H(\text{Parent}) = \text{SHA256}(C_1 \mathbin{\Vert} C_2)$
  * 3-node: $H(\text{Parent}) = \text{SHA256}(C_1 \mathbin{\Vert} C_2 \mathbin{\Vert} C_3)$
* **Verification Proof:** Verified in [test_auncient_23_tree.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tests/test_auncient_23_tree.c), demonstrating the demux path proof resolution of Leaf 2 inside a 3-node parent.
* **Bitcoin Script Implementation:** Script template at [acab_23_verify.script](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/btc_script/acab_23_verify.script) evaluates the witness stack to hash a 3-node sibling set (`L1` + `L2` + `L3`) using `OP_CAT` operations, confirming root validity.
