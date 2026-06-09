# Sharing a Coprocessor Between Multiple User CPUs

In a multi-user emulated environment (like our shared EVM smart contract hosting multiple 6502 guest threads), sharing a single coprocessor requires careful design. Depending on whether the coprocessor has **internal physical state** (e.g. a video display buffer) or is **stateless** (e.g. a math accelerator), different synchronization and isolation strategies are required.

This document analyzes how virtual coprocessors are multiplexed across user CPUs on-chain, detailing isolation layers and access control patterns.

---

## 1. Sharing Strategies

```
             ┌─────────── User A CPU (Writes input to Slot A) ───┐
             │                                                   ▼
             │                                        [Shared Coprocessor]
             │                                                   ▲
             └─────────── User B CPU (Writes input to Slot B) ───┘
```

### 1.1 Stateless Multiplexing (Namespace Isolation)
For computational coprocessors (like the **COMTAX** bracket calculator), the coprocessor does not need to store permanent internal state between cycles.
* **Mechanism:** Input and output registers are mapped to **User-Specific Memory Slots** using deterministic address hashing:
  $$\text{StorageSlot} = \text{keccak256}(\text{msg.sender}, \text{RegisterAddr})$$
* **Execution:** When User A triggers the calculation, the coprocessor reads from User A's unique slots, performs the math, and writes back to User A's output slots.
* **Concurrency:** Multiple users can invoke the coprocessor in the same block without any conflicts, as the EVM executes transactions sequentially and isolates storage namespaces.

### 1.2 Shared Hardware Mutex Locks
For coprocessors controlling a shared, stateful asset (e.g., a shared screen buffer, audio speaker, or parallel transmission line), concurrent writes would corrupt the shared output.
* **Mechanism:** A memory-mapped **Mutex Lock Register** is established (e.g., at `$D620` / `54816`).
* **Protocol:**
  1. **Acquire:** Before accessing the coprocessor, the CPU reads `$D620`. If it returns `0` (unlocked), the CPU writes `1` to lock it and stores its user ID as the owner.
  2. **Rejection:** If another user attempts to write to the coprocessor registers while `$D620` is locked by a different owner, the coprocessor immediately reverts the execution step.
  3. **Release:** When the owner finishes the transfer, they write `0` to `$D620` to free the lock.

### 1.3 Asynchronous FIFO Queuing
If reverting transactions is too disruptive, requests can be queued:
* **Mechanism:** The coprocessor maintains an on-chain Ring Buffer (FIFO queue).
* **Protocol:**
  * Users append their calculation payloads (e.g., coordinates to project) to the queue.
  * The coprocessor processes the queue sequentially, writing results back to each user's inbox mailbox registers.

---

## 2. Emulation Comparison

| Coprocessor Type | State Dependency | Isolation Technique | Concurrency Limit |
| :--- | :--- | :--- | :--- |
| **COMTAX Math** | Stateless | Deterministic Namespace Hash | Unlimited (parallel execution) |
| **Speech Synthesizer** | Stateful (audio channel) | Mutex Lock (`$D620`) | 1 owner per block/transaction |
| **IEEE-488 Parallel Bus** | Stateful (physical bus line) | Mutex Lock & Cooldown Timers | 1 owner per transfer session |
