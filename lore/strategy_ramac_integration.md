# Strategy Programming Language RAMAC Integration Layout

This document describes how the COBOL-style Strategy Programming Language compiler and runtime pipeline interface with all low-level hardware structures of the simulated RAMAC and STRETCH processors.

---

## 1. COBOL Arithmetic Mapping to RAMAC Accumulators
* **Syntax Translation:** The compiler translates statements like `ADD value TO acc` and `SUBTRACT value FROM acc` in [.strategy configuration files](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/strategies/sort_generator.strategy) directly into equivalent RAMAC ALU instructions (`ADD`, `SUB`).
* **Accumulator Binding:** These instructions are executed by the [tsfi_ramac_alu_exec](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c#L225) processor loop, operating on the [tsfi_ramac_acc_model](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_ramac_layout.h#L112) register arrays.

---

## 2. Strategy Data Layout on CHS Disk Platter
* **Cylindrical Hashing:** Strategy variables and BCD records are serialized to disk using the cylinder-overflow hashing algorithm (`tsfi_ramac_hash_key`).
* **Index Resolve:** Key-value lookups map directly to Cylinder-Head-Sector coords (`tsfi_ramac_index_to_chs`), and overflow entries are dynamically routed to overflow tracks (Heads 45-49).

---

## 3. SEC-DED ECC Memory Protection
* **Automatic Parity Encoding:** When a strategy program writes a 56-bit word to virtual storage, the runtime automatically encodes it using `tsfi_s370_ibm7030_ecc_encode`.
* **Syndrome Checking:** Reads fetch the 64-bit ECC word and run `tsfi_s370_ibm7030_ecc_decode` to transparently correct single-bit errors or trap double-bit errors at the VM layer.

---

## 4. Speculative Look-Ahead (LAU) Execution
* **Hazard Avoidance:** Pipelined strategy instruction executions are pushed to the `tsfi_ibm7030_lau_queue` Look-Ahead Unit.
* **RAW Store Forwarding:** Read-After-Write hazards are resolved inside the queue using speculative store forwarding before values commit to the strategy storage memory.

---

## 5. Parallel Multiprogramming via Traffic Control
* **Round-Robin Interleaving:** Multiple independent strategy files (such as `sort_generator.strategy` and `report_writer.strategy`) execute concurrently via the `tsfi_honeywell800_scheduler`.
* **Register Isolation:** Each strategy thread maintains isolated program counters and accumulator states, executing in interleaved instruction-cycle slices.
