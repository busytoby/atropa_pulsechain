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
* **Tax Due Registers (`$D5B2`–`$D5B5` / `54706`–`54709`)**: Stores the 32-bit calculated tax due value. This is computed dynamically via the COMTAX coprocessor as exactly 0.1% of the current remaining transaction gas (`div(gas(), 1000)`).
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

## 4. Multi-User CPU Security Review

The emulated CPU operates in a shared contract environment. Security is enforced through isolation layers:

### Sandbox Isolation
* **Address Namespace Separation**: User files, registers, and program states are strictly isolated using the `getUserSlot(addr)` function. Storage slots are derived deterministically:
  $$\text{Slot} = \text{keccak256}(\text{getContextUser()}, \text{addr})$$
  This prevents one user's emulation environment from reading or overwriting another user's variables, screen RAM, or directory records.

### CPU Re-entrancy and Block Debouncing
* **Switch Bounce Security**: The Disk System enforces block-level call debouncing. If a user calls the Disk System multiple times in the same block, it reverts, preventing flash-loan style re-entrancy attacks or double-spend exploits.
* **Dynamic Diyat Rate-Limiting**: To prevent off-chain scripts from spamming the `getCPUState()` selector within the same block, the CPU implements a gas-calibrated read penalty. If the selector is triggered multiple times in the same block (`number() == lastReadBlock`), a 2 unit Diyat tax is dynamically excised from the caller's balance and redirected to the treasury namespace, logging the penalty via `TaxPaid`.
* **CPU Privilege Isolation**: System-level calls (like changing the underlying disk system configuration) are restricted to the authorized deployer/master keys, preventing malicious emulated code from altering virtual hardware routes.

### Multi-User Inter-CPU Communication Security
* **Register Conflict Resolution**: The multi-user communication registers originally proposed at `$D5E0`–`$D5FF` have been remapped to `$D610`–`$D613` to resolve address range collisions with the 256-bit Cryptographic Nonce Generator.
* **Inter-User Interrupt Protection**: To prevent malicious actors from spamming interrupts (`IRQ`/`NMI`) to freeze other users' CPU states, three key security layers are designed:
  1. *Diyat Interrupt Tax*: Excises a 5 unit Diyat tax from the sender for each triggered interrupt.
  2. *Whitelisting*: Verifies that the recipient has whitelisted the sender in local RAM.
  3. *Cooldown Watchdog*: Imposes a minimum 3-block cooldown interval between successive inter-user signals.

### Resource Controls (Gas Limits)
* To prevent infinite loops (e.g. `JMP *`) from freezing the EVM node or draining gas, the CPU executes instructions in bounded batches (`runSteps(maxSteps)`). The maximum steps parameter acts as a hardware watchdog timer.
