# Micro Systems Development & Multi-User CPU: Tax & Security Analysis

This document details the virtual hardware upgrades implemented to support large nonce generation and JiffyDOS hook corrections, along with a design specification for tracking **Diyat Tax** payments on-chain and reviewing the **Security Architecture** of the multi-user shared CPU system.

---

## 1. Large Nonce Generator ($D5E0–$D5FF)

To allow the 8-bit emulated environment to handle cryptographic nonces and transaction salts, we implemented a memory-mapped 256-bit (32-byte) entropy source:

* **Control Register (`$D5E0` / `54752`)**: Writing a non-zero value to this register triggers the generation of a fresh 256-bit nonce.
* **Nonce Buffer (`$D5E0`–`$D5FF` / `54752`–`54783`)**: Reading this 32-byte range yields the individual bytes of the generated nonce.
* **EVM-Compatible Entropy Source**: The nonce is generated natively in Yul using:
  $$\text{Nonce} = \text{keccak256}(\text{block.number}, \text{block.timestamp}, \text{blockhash}(\text{block.number}-1), \text{user}, \text{counter})$$
  This leverages standard EVM opcodes (`number()`, `timestamp()`, `blockhash()`) ensuring universal compatibility across local networks and mainnets without relying on deprecated instructions like `PREVRANDAO`.

---

## 2. JiffyDOS & CPU Emulation Fixes

During integration testing, we resolved two critical emulation issues:
1. **Calldata Alignment**: Added the missing `mstore(0x324, totalCmdLen)` offset prefix inside the `$FFD5` (LOAD) hook inside [cpu6502.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/cpu6502.yul), preventing decoding reverts.
2. **JSR Stack Correction**: Modified the JSR (`$20`) instruction to push `currentPC + 2` to the stack (matching standard 6502 hardware behavior) instead of `currentPC`. This ensures that when JiffyDOS routines like LOAD or SAVE complete and pull the return PC from the stack, they resume at the next valid instruction instead of executing unhandled operands as instructions (which previously triggered CPU reverts).

---

## 3. Diyat Tax Payment Tracking System ($D5CF)

To track Diyat taxes and maintenance fees received by the operating platform, we implemented a dedicated **Treasury and Double-Entry Ledger** pattern directly mapped to CPU memory:

### Register Layout
* **Tax Due Registers (`$D5B2`–$D5B5 / `54706`–`54709`)**: Stores the 32-bit calculated tax due value. This is computed dynamically via the COMTAX coprocessor as exactly 0.1% of the current remaining transaction gas (`div(gas(), 1000)`).
* **Tax Payment Trigger Register (`$D5CF` / `54735`)**: Writing a non-zero value to this register triggers the tax payment sequence:
  1. Reconstructs the 32-bit `taxDue` from the registers (derived dynamically from the transaction's remaining gas).
  2. Verifies the taxpayer's OTRT (Reward Token) emulated balance mapped at RAM address `848`.
  3. Deducts `taxDue` from the user's balance.
  4. Credits `taxDue` directly to the system treasury balance (mapped to treasury address namespace at RAM address `848`).
  5. Clears the calculated `taxDue` registers.
  6. Writes success status `1` back to register `54735`.
  7. Emits a `TaxPaid` EVM event log for historical ledgers.

### Event Logging and Double Entry
Taxes collected by the system are journaled on-chain via EVM logs:
```solidity
event TaxPaid(address indexed taxpayer, uint256 amountPaid);
```
Topic1: `keccak256("TaxPaid(address,uint256)")` = `0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925`.

We have fully verified this mechanism using the unit test [test_cpu_tax.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_cpu_tax.js).

### Parallel Bus Diyat Tax ($D5B7)
To align virtual hardware speed optimizations with the resource model, utilizing the high-speed **Parallel Bus / IEEE-488 Coprocessor** triggers an automatic transaction tax:
* **Tax Calculation**: Calculated dynamically as `1` unit per `64` bytes transferred plus a base fee of `1` unit:
  $$\text{Bus Tax} = \text{floor}(\frac{\text{Length}}{64}) + 1$$
* **Automatic Excise & Enforcement**: Before reading or writing, the CPU checks the user's OTRT balance. If the balance is less than `busTax`, the transaction reverts. Otherwise, the tax is dynamically deducted, credited to the platform treasury, and recorded via the `TaxPaid` event log.

### JiffyDOS KERNAL Hook Diyat Tax ($FFD5 & $FFD8)
To prevent exploitation of high-speed bypass operations that skip the physical IEC serial loops, running the **JiffyDOS KERNAL Hooks** enforces a mandatory transaction fee:
* **Hook Tax**: A flat fee of **5 OTRT units** is charged for either `LOAD` (`$FFD5` / `65493`) or `SAVE` (`$FFD8` / `65496`) vectors.
* **Enforcement**: If the executing taxpayer has less than 5 units of OTRT in RAM balance, the instruction execution immediately reverts. Otherwise, the fee is deducted and journaled to the treasury.

### Web UI Real-Time Collection Log
To enable operator tracking of all received payments, the frontend parses the `TaxPaid` event logs:
1. Queries the EVM node for logs matching the `TaxPaid` signature hash.
2. Dynamically decodes the taxpayer address (from topic 1) and payment amount (from data).
3. Lists the entries chronologically in a scrolling log panel, ensuring full transparency of incoming platform collections.

---

## 4. Multi-User CPU Security & Sandbox Isolation Architecture

The emulated CPU operates in a shared contract environment. Security, containment, and fault isolation are enforced through dedicated mechanisms:

### A. Sandbox Isolation and Storage Namespace Separation
In a multi-user smart contract, preventing cross-user state corruption is critical. We achieve complete sandbox containment using deterministic storage virtualization:
* **Mathematical Boundary Derivation**: Instead of mapping the emulated 64KB memory map directly to contiguous EVM storage slots, every memory access is routed through a hash-based slot calculator:
  $$\text{Slot} = \text{keccak256}(\text{getContextUser()}, \text{address})$$
* **Proof of Non-Overlap**: For two distinct users $U_1 \neq U_2$ and any two memory addresses $A_1, A_2$, the collision resistance property of $\text{keccak256}$ guarantees that:
  $$\text{keccak256}(U_1, A_1) \neq \text{keccak256}(U_2, A_2)$$
  Therefore, it is cryptographically impossible for User 1 to read, write, or corrupt the memory pages, stack frames, program counters, or registers of User 2.

### B. Block-Level Re-entrancy and Switch-Bounce Debouncing
To prevent flash-loan-assisted recursive calls and re-entrancy vulnerabilities in the Disk System:
* **Caller Debouncing**: The system maintains a `lastCallerBlock` mapping in storage.
* **Enforcement**: If any address initiates a call to the disk system or memory controllers while the transaction block is active and `block.number == lastCallerBlock`, the execution is aborted immediately. This neutralizes multi-transaction re-entrancy and state-manipulation loops.

### C. Dynamic Diyat Rate-Limiting & Read Penalties
To mitigate off-chain client spam (e.g., automated scripts flooding the contract with `getCPUState()` calls to extract real-time register states), the system implements an active gas-calibrated read penalty:
* **Read Tracker**: The CPU tracks the block number of the last state extraction.
* **Excise Penalty**: If the selector is queried multiple times within the same block, a **2 OTRT unit Diyat penalty** is debited from the caller's balance, transferred to the treasury namespace, and logged.

### D. CPU Privilege Ring Isolation
The emulated 6502 architecture executes code under virtual privilege rings, distinguishing between User programs and Supervisor routines:
* **Supervisor Mode (Ring 0)**: Allowed to access physical device registers, alter disk mounting options, and adjust communication channels.
* **User Mode (Ring 1)**: Executed programs cannot directly write to hardware control registers. Attempts to write to forbidden zones (such as rewriting the JiffyDOS kernel vectors or system disk layouts) trigger a virtual CPU fault, returning execution back to the supervisor shell.

### E. Interrupt Sanitization & Inter-User Signalling Protection
Users communicate and dispatch hardware interrupts (`IRQ`/`NMI`) to other users through mapped signal registers at `$D610`–`$D613`. To prevent malicious actors from freezing target CPU states via signal storms, three mitigations are active:
1. **Diyat Signal Surcharge**: Triggering an external interrupt costs **5 OTRT units** per signal, discouraging high-frequency spam.
2. **Whitelisting**: The recipient user must explicitly register the sender address in their local whitelist table; unsolicited interrupts are automatically discarded.
3. **Cooldown Watchdog**: A minimum 3-block cooldown interval is enforced between signals. Any signal dispatched before the cooldown period expires is dropped without execution.
