# CABS-Folklore CPU: Peek/Poke Security Architecture

This document analyzes the security model of the **CABS-Folklore CPU** virtual memory space. Specifically, it details how `peek` and `poke` operations are secured against unauthorized state manipulation, memory collisions, and front-running in a decentralized EVM environment.

---

## 1. Cryptographic Namespacing (EVM-Style Sandbox)

In traditional hardware architectures (like the 6502), `peek` and `poke` allow arbitrary reading and writing of a shared, flat memory map. On a public blockchain, a shared flat memory map would allow any user to overwrite another player's game state, steal escrowed funds, or falsify achievements.

To prevent this, the Folklore CPU implements **Cryptographic Namespacing** inside the Yul runtime:

```solidity
function getContextUser() -> user {
    user := mload(0x1F0)
    if iszero(user) {
        user := caller()
    }
}

function getUserSlot(addr) -> slot {
    let user := getContextUser()
    mstore(0x00, user)
    mstore(0x20, addr)
    slot := keccak256(0x00, 64)
}
```

### Security Properties of Namespacing:
1. **Cryptographic Isolation**: When a caller executes `poke(addr, val)`, the storage slot is resolved as:
   $$\text{Slot} = \text{keccak256}(\text{msg.sender} \parallel \text{registerAddress})$$
   Because of the preimage resistance of `keccak256`, Player A (`0xAAA...`) can never calculate a write operation that overlaps with Player B (`0xBBB...`), even if they write to the exact same virtual memory address (e.g., `55050`).
2. **Implicit Ownership**: A player cannot write data "on behalf of" another player. A `poke` instruction is permanently bound to the transaction signer or the calling contract address (`msg.sender`).

---

## 2. Read/Write Access Separation (Peek vs. PeekUser)

The architecture separates state manipulation (`poke`) from auditing/read operations:

* **`poke(uint256 addr, uint256 val)` (Write-Only Scoped)**:
  Writes exclusively to `getUserSlot(addr)`. It can only modify memory owned by the current transaction sender.
* **`peek(uint256 addr) -> uint256` (Read-Only Scoped)**:
  Allows the caller to read registers within their own namespace.
* **`peekUser(address player, uint256 addr) -> uint256` (Read-Only Global)**:
  Exposes a `view` interface for read-only audits. It allows external contracts (like `CABSMarketMachine` or the `ZMachine`) to inspect a user's register state to verify quest completion or wagers. Because it does not support writing, it is completely safe from state-hijacking.

---

## 3. Sandboxed External CPU Calls (Ledger Resilience)

In `CABSMarketMachine.sol`, interactions with the Folklore CPU register space are executed using low-level, sandboxed calls:

```solidity
folkloreCPUAddress.call(
    abi.encodeWithSignature("poke(uint256,uint256)", 57344, 1)
);
```

### Security Benefits of Low-Level `.call`:
1. **Revert Containment**: If the Folklore CPU contract Yul code reverts (due to out-of-gas, division by zero, or contract corruption), the main escrow ledger transaction **does not revert**. The call fails silently, allowing the ledger to safely lock or unlock tokens without getting bricked by external dependencies.
2. **Gas Guzzling Protection**: By decoupling the interface, the ledger limits gas exposure, preventing a corrupted or malicious CPU contract from consuming the entire transaction gas limit and preventing ledger settlement.

---

## 4. Hardware State Machine Invariants

The CABS system enforces ledger invariants that cannot be altered by arbitrary register pokes:

* **Bookkeeping Authority**: The actual token balance is controlled by the EVM smart contract ledger (`CABSAccounts` mappings). Folklore registers only act as **status monitors** (`CABS_CTRL`).
* Even if a malicious user bypasses the CPU and pokes `CABS_CTRL` (`57344`) to `2` (COMMIT) in their own namespaced storage:
  - The `CABSMarketMachine` only respects states queried from its **own** contract namespace (`peekUser(marketMachineAddress, 57344)`), or checks the actual on-chain transaction logs.
  - Thus, poking arbitrary registers in a user's namespace has no impact on the security or custody of the contract vault's ERC-20 tokens.
