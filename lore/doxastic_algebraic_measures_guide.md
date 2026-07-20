# Programmable Algebraic Measures Guide

This document details the mathematical and VM structures for treating programmable algebras as continuous probability measures over coordinate spaces.

```
       [ Event Weights (DAT Rails) ]
                     |
                     v
   [ Algebraic Intersection Ring P(A * B) ]
                     |
                     v
[ VM OP_PROB_TRY Stochastic Sampling Check ]
                     |
      +--------------+--------------+
      | (Roll >= Threshold)         | (Roll < Threshold)
      v                             v
[ Satisfied Target (PC Jump) ]   [ Choice Point Backtrack ]
```

---

## 1. Algebraic Representation

We define probability measures over our 3D coordinate space by mapping paths to numeric weights in a polynomial ring:
*   **Coordinate Weight**: A value $W$ stored at a path (e.g. `events/owns_token`) represents the likelihood weight $P(E) = W / 100$.
*   **Ring Structure**: The state space is modeled as a commutative algebra where coordinates are independent polynomial variables.

---

## 2. Joint Probabilities as Intersection Rings

Logical operations are evaluated algebraically as operations on coordinates:
*   **Logical Intersection (AND)**: Evaluated as polynomial multiplication. The joint probability of independent coordinates $A$, $B$, and $C$ is:
    $$P(A \cap B \cap C) = P(A) \cdot P(B) \cdot P(C)$$
*   **Logical Union (OR)**: Evaluated as:
    $$P(A \cup B) = P(A) + P(B) - P(A) \cdot P(B)$$

This reduces multi-variable logical checks to a single coordinate multiplication, saving execution cycles.

---

## 3. VM Execution & Branching

Stochastic branching is evaluated via the `OP_PROB_TRY` VM instruction:

1.  **Load Threshold**: The joint probability threshold is pushed onto the stack.
2.  **Evaluate Branch**: `OP_PROB_TRY <target_pc>` queries the VM's LCG pseudo-random generator:
    *   If `roll >= threshold`, the branch is satisfied, pushing a choice point and continuing execution.
    *   If `roll < threshold`, execution jumps directly to the alternative target PC.
3.  **Backtrack**: If a constraint fails later in execution, the `OP_FAIL` opcode pops the choice point, restoring the stack and unbinding trail variables.

---

## 4. Verification Coverage

*   [test_algebraic_measures.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_algebraic_measures.c): Validates the retrieval of independent weights from the DAT rails, joint measure evaluation ($16\%$), and correct backtracking branching via `OP_PROB_TRY`.
