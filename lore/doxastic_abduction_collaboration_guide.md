# Collaborative Abduction & Voxel Classification Guide

This guide documents the integration of **voxel strain classifiers** with **distributed peer abduction challenges**, enabling the logic network to identify boundaries of ignorance and resolve them collaboratively.

```
+--------------------------------------------------------------+
|                     The Collaboration Loop                   |
+-----------------------------------+--------------------------+
                                    | (Classifier detects Front)
                                    v
+--------------------------------------------------------------+
|                  Boundary: CLASS_FRONTIER                    |
|        (Crystallized Fact adjacent to Pending Question)      |
+-----------------------------------+--------------------------+
                                    | (Publishes challenge)
                                    v
+--------------------------------------------------------------+
|               Challenge: challenge/peer_abduction            |
|        (Registered on Anvil and compiled to local DAT)        |
+-----------------------------------+--------------------------+
                                    | (Peer solves and verifies)
                                    v
+--------------------------------------------------------------+
|                   Crystallized Fact: RELATION_TRUE           |
|        (DAT slice updated, strain drops to CLASS_STABLE)      |
+--------------------------------------------------------------+
```

---

## 1. Doxastic Strain Gradients

The logic agent models its coordinate space using physical strain metaphors. Strain is defined as the rate of change of logical values between adjacent coordinate voxels:

*   **Low Gradient**: Adjacent cells share matching values (`RELATION_TRUE` next to `RELATION_TRUE`), representing logical consensus (**Stable**).
*   **High Gradient**: A crystallized fact sits next to a pending question (`QUESTION_PENDING`), representing a boundary of active learning (**Frontier**).
*   **Fractured Gradient**: A crystallized fact sits next to a retracted/unbound void, representing logical contradiction (**Fracture**).

---

## 2. Boundary Classification States

The strain classifier (`tsfi_doxastic_classify_strain`) scans coordinate segments and maps the boundary gradient to one of three classes:

| Class | ID | Description | Resolution Path |
| :--- | :--- | :--- | :--- |
| `CLASS_STABLE` | `10` | Low strain gradient. Consensual fact distribution. | None required. |
| `CLASS_FRONTIER` | `11` | High strain gradient. Fact adjacent to `QUESTION_PENDING`. | Trigger abduction check or publish peer challenge. |
| `CLASS_FRACTURE` | `12` | High strain gradient. Fact adjacent to empty/retracted space. | Trigger unbinding backtracking or rule damping. |

---

## 3. Dynamic Peer Abduction Loop

When local reasoning cannot resolve a `CLASS_FRONTIER` boundary, the agent initiates the decentralized challenge loop:

1.  **Publish**: The local VM calls `sys_write` to register a unique challenge hash at `challenge/peer_abduction/<hash>` as `ACTIVE`.
2.  **Advertise**: The local sync daemon broadcasts the challenge to the network via MCP.
3.  **Solve**: A peer solver provides the missing fact proof, writing `RELATION_TRUE` to the path and setting the challenge status to empty.
4.  **Consolidate**: The local sync daemon re-compiles the DAT rails. The classifier verifies the boundary strain has dropped, shifting the region classification from `CLASS_FRONTIER` to `CLASS_STABLE`.

---

## 4. Verification Coverage

The complete classification and collaboration cycle is verified by the following integration test targets:

*   [test_strain_classifier.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_strain_classifier.c): Verifies correct boundary classification outputs for `CLASS_FRONTIER` and `CLASS_FRACTURE`.
*   [test_doxastic_rules.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_doxastic_rules.c): Verifies that classified regions transition from `CLASS_SEED` to `CLASS_BOND` post successful VM constraint evaluation.
*   [test_doxastic_loop.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_doxastic_loop.c): Verifies the closed-loop stabilization cycle (frontier detection $\rightarrow$ abduction check $\rightarrow$ crystallization $\rightarrow$ stabilization).
*   [test_peer_abduction.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_peer_abduction.c): Verifies challenge publishing, peer resolution, and local consolidation.
