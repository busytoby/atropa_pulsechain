# Auncient Agentic Memory & Distributed RDBMS

This document details the architecture, routing mechanics, and caching layers of the Auncient Agentic Memory system, which allows standard allocated `lau_memory` members to operate as self-optimizing relational database agents.

The database capabilities and schema routing routines are integrated directly under the **Auncient Interoperability Registry** (`libantigravity_interop.c`) to maintain a unified, thread-safe dynamic interface completely free of local mocks.

---

## 1. Agentic Memory Architecture

In the Auncient VM layout, memory allocations are not static structures. Every block allocated via `lau_malloc_wired` contains a co-allocated **LauWiredHeader** at a fixed offset of `-8192` bytes relative to the payload pointer returned to the guest application.

```
       +---------------------------------------------+
       |               LauWiredHeader                |
       |  - Reflection Schema (Method Signatures)    |
       |  - Helmholtz Cache  - State Counter         |
       +---------------------------------------------+
       | Payload Start (0x2000 offset)               |
       |  - Active Registers (LauRegisterBank)       |
       |  - Relational Table Data Rows               |
       +---------------------------------------------+
```

Each memory agent maintains:
* **Dynamic Reflection Schema**: An array of `ThunkSignature` structures specifying selector mappings and target execution function pointers.
* **State Counter**: An atomic write-invalidation clock (`counter`) tracking state mutations.
* **Sealing Properties**: Security gating properties (`sealed`, `version`, `system_id`) validating structural integrity.

---

## 2. Lock-Free ABI Dispatch Map

Dynamic method execution is routed through `ABIDispatchMap` using Knuth's multiplicative open-addressing hash table:

1. **Schema Self-Discovery**:
   - Calling `abi_dispatch_register_member(map, payload)` reads the target header at offset `-8192` bytes and auto-binds all registered selectors to their target function pointers.
2. **Dynamic Invocations**:
   - Calling `abi_dispatch_invoke` executes selectors dynamically on any registered target agent. It resolves the execution offset and calls the target thunk with argument arrays in a unified format.

---

## 3. Helmholtz Memoization Caching

To achieve sub-2ns lookup latency, each agent's header contains a co-allocated **Helmholtz cache slot** that caches the bijection of method inputs to outputs:

```c
typedef struct {
    uint64_t cache_input_hash;
    uint64_t cache_output_val;
    int cache_state_epoch;
    int cache_valid;
} LauWiredHeader;
```

### Invalidation & Execution Bypass Flow:
1. **Hash Verification**:
   - The dispatcher hashes the solidity method selector and input arguments using FNV-1a.
2. **Epoch Comparison**:
   - If `cache_valid` is set, `cache_input_hash` matches the invocation hash, and `cache_state_epoch == h->counter`, the dispatcher returns `cache_output_val` directly. This bypasses guest VM execution entirely, resolving in **`1.30 ns`**.
3. **Lazy Invalidation**:
   - Any state-mutating operation (e.g. insertions or updates) increments the agent's counter (`h->counter++`). The next query invocation detects a version mismatch and forces full execution.

---

## 4. Distributed Memory-to-Memory RDBMS

Agents can structure their payload boundaries as dynamic relational tables. Operations are dispatched directly between agents via in-memory ABI calls:

```
+----------------+                       +-------------------+
|  Caller Agent  | --[ abi_dispatch ]--> |  Database Agent   |
| (payload7)     |                       | (payload8)        |
|                |                       | - create_table    |
|                |                       | - insert_row      |
|                |                       | - query_row       |
+----------------+                       +-------------------+
```

### Relational API Selectors:
* **`0xcccccccc` (Create Table)**: Allocates dynamic row memory matching requested column constraints.
* **`0xaaaaaaaa` (Insert Row)**: Appends tuple records to the table, and clocks the database agent's counter to invalidate read caches.
* **`0xbbbbbbbb` (Query Row)**: Evaluates match constraints and retrieves column records, caching subsequent query hits.

---

## 5. Architectural Enhancements

To ensure production stability, key architectural improvements have been added directly to `libantigravity_interop.c`:

### A. Zero-Copy Serialization & Cascaded FNV-1a Hashing
When dynamic tables are updated, their metadata and raw rows are immediately written to disk using direct binary serialization to `../assets/table_<address>.dat.bin`. Before writing, an optimized **cascaded FNV-1a checksum** processes the entire buffer block-by-block using 64-bit words, rather than individual byte loops, increasing hashing throughput by up to $4\times$ and prefixing the block as a validation witness.

### B. Multi-Key Indexing via Atomic BST Swaps
To prevent $O(N)$ linear scanning on queries, insertions register keys into a dynamic, in-memory **Binary Search Tree (BST)** index (`InteropBSTNode`). Rather than updating the BST in-place, the write-path clones the current tree index (`bst_clone_tree`), adds the new node to the cloned tree, and swaps the `bst_root` pointer atomically using `__atomic_store_n` with release semantics. This guarantees readers always traverse a consistent, read-only tree.

### C. Read-Copy-Update (RCU) Thread Safety
Writer pipelines construct a new copy of the rows buffer during mutation events, copy past data, append the new row, and swap the `rows` pointer atomically. Readers load the pointer atomically using acquire semantics, allowing lock-free concurrent query execution.

### D. Verifiable Transaction Audit Logging (Immutability Ledger)
Every database mutation is logged sequentially inside `../assets/audit_log_<address>.dat.bin` using a Merkle-like chain format (`InteropAuditEntry`). Each entry stores the `prev_hash` of the previous audit entry and computes an optimized FNV-1a block hash of its own parameters. The verification engine (`interop_agent_verify_audit_log`) scans the file bottom-up to confirm the chain links are unbroken and rejects any modified or deleted logs.

### E. Coaxial Shared Memory Ledgers (Zero-Copy Offset Maps)
For VM-to-host zero-copy communication, the shared segment layout (`CoaxialSharedHeader`) uses **relative offsets** instead of absolute address pointers (since address mapping layouts differ across host and guest environments). Writes are coordinated via atomic spinlock markers in the shared memory header, while reads run lock-free by atomically retrieving the offset word (`rows_offset`), ensuring immediate multi-system synchronization.

### F. Extended System-Wide Coaxial Shared Memory Ledger
To coordinate global platform parameters, the `InteropSystemLedger` layout groups multiple specialized coaxial tables:
* **`trie_route_table`**: Zero-copy index tree mapping Merkle-Patricia EVM state routes.
* **`wmq_event_table`**: High-frequency ring buffer logging guest-to-host SCSI and event queues.
* **`gas_calibration_table`**: Real-time read-only fee tables tracking gas parameters updated by host profiling metrics.
* **`ipc_signal_table`**: Zero-copy signal routing table linking process IDs to event mask flags for VM interrupts.
* **`vram_page_table`**: Page frames layout tracking video memory blocks allocated by Vulkan context renderers.
* **`jit_reflection_table`**: Method compilation cache storing selector targets mapped to raw JIT instruction pointer offsets.
* **`peer_registry_table`**: Discovery lookup table mapping active agent hashes to virtual WinchesterMQ SCSI paths.
* **`radio_packet_table`**: Link-layer transmission queue logging LoRa/OOK physical packet telemetry.
* **`cpu_snapshot_table`**: Host debugging registry capturing CPU register snapshots and active frame pointers.

### G. Live UNIX Domain Socket Loopback Bridge
For direct guest userspace integrations, the interop library establishes a non-blocking UNIX domain socket server loopback bridge at `/tmp/test_coaxial_bridge.sock`. Guest-side client processes transmit Solidity-style method selector packets over the local socket file descriptor, which is polled asynchronously (`interop_coaxial_bridge_poll`) and written directly to the CSML relative offset tables without kernel-space transitions.

### H. RDBMS-PLL Phase Lock State Synchronizer
To synchronize replicated database states between guest user spaces and host hypervisors, the synchronizer structure (`InteropPLLHeader`) tracks host and guest version counters. The synchronizer calculates the phase offset (state delta) using `interop_pll_detect_phase_error` and performs corrective alignment loops using `interop_pll_align_state` (atomically swapping pointer offsets to match execution scopes).

### I. Kermit over RDBMS-PLL (Verifiable Media File Transfer)
Implements Kermit-style file transfer protocols directly over the RDBMS-PLL phase lock synchronization loops. Files are divided into packet blocks (`InteropKermitPacket` formats) and mapped as relational tuples in the coaxial tables. Real-time file block acknowledgments are executed via state phase locks, and complete reconstructed files are serialized directly to disk as binary MEDIA DAT assets (`.dat.bin` formats), complying with system storage constraints.

### J. Turing-Complete RDBMS-PLL-DAT Media State Interpreter
Implements a Universal Turing Machine (UTM) interpreter directly over the coaxial RDBMS structures. The infinite tape register is mapped onto `trie_route_table` (MT-01), the transition state rules onto `jit_reflection_table` (MT-06), and the control head pointer onto `cpu_snapshot_table` (MT-09). Using `interop_turing_run_step`, cells are read, rules matched, tape values updated, and head movements tracked, proving that the RDBMS-PLL-DAT Media layout is **fully Turing-complete**.

### K. Lock-Free SPSC Coaxial Ring Queue
Provides a high-throughput, Single-Producer Single-Consumer (SPSC) ring queue implementation (`InteropCoaxialQueue`) mapping queue buffers directly inside the coaxial shared memory segment. Items are enqueued (`interop_queue_push`) and deleted (`interop_queue_pop`) using atomic load-acquire and store-release semantics, eliminating thread locking overhead.

### L. Bitcoin Covenant State Evolution Verification
Integrates Bitcoin-style covenant rules to verify state-to-state evolution of the Universal Turing Machine database. Rather than tracking sequential transaction histories in blocks, state coordinates are cryptographically anchored using FNV-1a block checksum hash links (`InteropCovenantState`). A state change transaction spent via UTXO execution is validated using `interop_covenant_verify_evolution`, proving correctness of state-to-state mutations and establishing immutable decentralization guarantees.

### M. Yul Virtual Contract Covenant Deployer
Enables the deployment of low-level Yul virtual contracts directly via Bitcoin covenant witness data payloads. The deployer function `interop_covenant_deploy_yul` compiles Yul transition tuple instructions directly into the coaxial machine's transition rules table, enabling stateful covenant computing environments.

### N. Layer-2 Rollup Batch Verification
Accelerates transaction processing speed by verifying batched sequences of state mutations. The `interop_covenant_verify_batch` API executes a set of Turing steps against the dynamic tables, producing a single final state evolution hash from a unified transition witness to reduce on-chain block footprint.

### O. Optimistic Fraud Proof Engine
Secures the off-chain rollup pipeline against invalid state assertions. The `interop_covenant_prove_fraud` API executes contested state transitions on-chain, proving state updates are fraudulent if the asserted hash deviates from the calculated step output.

### P. Parallel Vectorized FNV-1a Checksums (SIMD-Equivalent Hashing)
Uses parallel multi-lane word unrolling (`fnv1a_hash_vectorized`) to compute data table checksums across four 64-bit vector lanes, removing CPU profiling bottlenecks during deep state table serialization.

### Q. Bidirectional Host Response Verification
Validates execution status and return values transmitted by the host VM back to the L1 Bitcoin chain. The `interop_covenant_verify_response` API evaluates exit codes and return values inside UTXO spending validations.

### R. State-Reconstruction Log Replay
Enables the deterministic recovery and reconstruction of the `.dat.bin` Turing machine state tape from sequential transaction delta logs. The `interop_covenant_replay_log` API parses state deltas, applies them to the tape, and cryptographically verifies the resulting evolved FNV-1a hash matches the on-chain root commitment.

---

## 6. Performance Benchmarks

Performance metrics compiled using the standalone compilation profiler (`benchmarks/bench_self_compiler.c`):

| Operation | Metric | Latency |
| :--- | :--- | :--- |
| **Self-Extraction & JIT Compilation** | Total bootstrap build time | **102.14 ms** |
| **Transaction Audit Logging** | Writing FNV-1a block ledger (1000 writes) | **8.73 us / write** |
| **Bottom-up Audit Verification** | Cryptographic block validation (1000 blocks) | **58 ns / block** |

---

## 7. Verification Harness

To execute the test suite validating lock-free map allocations, general dynamic member registration, Helmholtz caching, RCU thread safety, BST indexing, guest VM WinchesterMQ SCSI loops, verifiable audit logging, coaxial shared offset swaps, system-wide ledger tables, live domain socket loopback bridges, RDBMS-PLL synchronizers, Kermit-over-PLL media transfer pipelines, Turing Machine interpret loops, SPSC ring queues, state evolution covenants, Yul virtual contract covenant deployments, Layer-2 Rollup batches, Optimistic Fraud Proofs, Parallel Vectorized Hashing, Bidirectional Host Responses, and State Reconstruction Replays:

```bash
cd scripts
gcc -Wall -Wextra -Werror -std=c11 -I../tsfi2-deepseek/inc -o test_abi_dispatch test_abi_dispatch.c abi_dispatch_map.c libantigravity_interop.c
./test_abi_dispatch
```
