# Doxastic Logic Programming Guide

This document defines the architecture, stack virtual machine opcodes, double-array trie (DAT) storage mappings, and classification mechanisms driving the agent's logic programming framework.

```
       [ Logic Rules / SVDAG Metaphor ]
                     |
                     v
   [ Stack VM (Opcodes / Choice Points) ]
                     |
      +--------------+--------------+
      |                             |
      v                             v
[ WAM Trail Stack ]         [ DAT Trie Rails (.dat.bin) ]
(Backtrack unbinding)       (Sub-microsecond fact lookup)
```

---

## 1. Stack VM Opcode Specifications

The stack virtual machine executes logical programs using a set of dedicated logical and control-flow instructions:

| Opcode | Mnemonic | Description | Stack Pre-conditions | Post-conditions |
| :--- | :--- | :--- | :--- | :--- |
| `0x21` | `TRY_ME_ELSE` | Pushes a backtracking choice point pointing to an alternative execution PC. | None | Choice Point pushed |
| `0x22` | `FAIL` | Forces a backtrack. Pops the latest choice point, restores stack state, unbinds variables via the Trail, and jumps to the target PC. | Latest Choice Point | PC updated to target, variables unbound |
| `0x24` | `NEQ` | Asserts inequality. If the two popped values are equal, triggers backtrack (`FAIL`). | `[val1, val2]` | Pushes success token if unequal, else fails |
| `0x25` | `BIND` | Binds a value to a stack variable index, registering the binding on the Trail stack. | `[value, var_index]` | `stack[var_index] = value` |
| `0x26` | `PROB_TRY` | Evaluates a probabilistic branch. Rolls an LCG pseudo-random value. If `roll >= threshold`, pushes a choice point and continues. If `roll < threshold`, jumps directly to the target PC. | `[threshold]` | Choice Point pushed or PC jumped |

---

## 2. Double-Array Trie (DAT) Rails Storage

All facts, beliefs, and open questions are stored in a memory-mapped Double-Array Trie database (`.dat.bin`), optimizing lookup latency to **under 50 ns**.

*   **Fact Paths**: Ground facts map to paths inside the trie (e.g. `svdag/x/y/z` maps to `"RELATION_TRUE"`).
*   **Pending Questions**: Open questions map to paths with value `"QUESTION_PENDING"`, indicating high-potential strain fields.
*   **Retractions**: Retracted facts are zeroed out (value set to `""`), removing them from matching query lookups.

---

## 3. Doxastic Region Classification

To analyze logic structures on the fly, the **Doxastic Classifier** (`tsfi_doxastic_classify_region`) scans 3D subgrids in the SVDAG, classifying the region density:

*   **`CLASS_SEED`** (Potential): The region is dominated by pending questions (`QUESTION_PENDING` density $> 50\%$). The agent focuses its curiosity here to resolve open questions.
*   **`CLASS_BOND`** (Crystallized): The region is dominated by verified, immutable facts (`RELATION_TRUE` density $> 50\%$).
*   **`CLASS_VOID`** (Empty): The region is dominated by empty or retracted spaces (density $> 50\%$).

---

## 4. Closed-loop Crystallization Loop

When a pending question gets answered:
1.  The sync daemon detects the verification on Anvil.
2.  `tsfi_dat_compile_relation` overwrites the `"QUESTION_PENDING"` path to `"RELATION_TRUE"`.
3.  The SVDAG spring strain collapses to zero, and the fact is permanently crystallized on the DAT disk rails.
