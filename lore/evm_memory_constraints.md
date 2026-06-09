# Developing under Memory and Storage Constraints on the EVM

Like the 5 KB memory limitation of the Commodore VIC-20, the Ethereum Virtual Machine (EVM) imposes severe constraints on memory, stack depth, and persistent storage. However, while retro computers were limited by physical hardware, the EVM is constrained by **economic gas costs** and **virtual architecture limits**.

This guide explains the EVM's memory, storage, and stack models, detailing how to optimize your code to build gas-efficient and functional smart contracts.

---

## 1. EVM Memory Architecture & Constraints

The EVM has three main areas for holding data:

| Memory Area | Persistence | Cost | Access Size | Characteristics |
| :--- | :--- | :--- | :--- | :--- |
| **Stack** | Ephemeral | Very Cheap | 32 bytes | LIFO stack, maximum depth of 1024 elements. |
| **Memory** | Ephemeral (Transaction lifetime) | Quadratic expansion | Byte-addressable | Linear, volatile memory array. |
| **Storage** | Permanent (On-chain) | Extremely Expensive | 32-byte slots | Key-value store mapping slots to values. |

### The Quadratic Memory Expansion Cost
EVM memory is a linear, byte-addressable array initialized to zero. While writing to memory is initially cheap, the gas cost for memory expansion scales **quadratically** once you exceed 724 bytes (23 words).

The gas cost formula for expanding memory to $N$ words (32-byte chunks) is:

$$\text{Gas}_{\text{memory}} = 3 \cdot N + \lfloor \frac{N^2}{512} \rfloor$$

> [!WARNING]
> Because of the quadratic $N^2$ term, large memory allocations (e.g., dynamically sized arrays, large string concatenations, or deep recursions) quickly become cost-prohibitive and can hit block gas limits.

---

## 2. Ephemeral Memory Optimization Patterns

### Use `calldata` Instead of `memory`
For read-only function parameters (especially arrays, structs, and strings), declaring them as `calldata` instead of `memory` avoids copying the data from transaction inputs into memory.

```solidity
// Avoid: Copies array into volatile memory
function processData(uint256[] memory items) external pure returns (uint256) {
    return items[0];
}

// Preferred: Reads directly from transaction payload without memory expansion
function processData(uint256[] calldata items) external pure returns (uint256) {
    return items[0];
}
```

### Memory Reuse & Scratch Space
In Yul assembly, the first 64 bytes (`0x00` to `0x3f`) are designated as **scratch space** for hashing methods. Using these slots for temporary operations avoids expanding the free memory pointer (located at `0x40`).

```yul
// Storing temporary variables in scratch space instead of expanding memory
let temp := sload(someSlot)
mstore(0x00, temp)
let hash := keccak256(0x00, 32)
```

---

## 3. Storage Optimization & Tight Variable Packing

Storage is the most expensive resource on the EVM. A single write (`SSTORE`) can cost up to 20,000 gas.

### Tight Variable Packing (Solidity)
Solidity groups sequential variables into the same 32-byte slot if their combined size fits. Always order variables to maximize packing density.

```solidity
// Bad Layout: Takes 3 storage slots (3 x 32 bytes)
struct UserBad {
    uint8 age;       // Slot 0 (1 byte)
    uint256 balance; // Slot 1 (32 bytes - cannot pack with uint8)
    uint8 level;     // Slot 2 (1 byte)
}

// Packed Layout: Takes 2 storage slots (1 slot saved!)
struct UserGood {
    uint8 age;       // Slot 0 (1 byte)
    uint8 level;     // Slot 0 (1 byte)
    uint256 balance; // Slot 1 (32 bytes)
}
```

### Transient Storage (`TSTORE`/`TLOAD`)
Introduced in EIP-1153 (Cancun upgrade), transient storage behaves like normal storage but is cleared at the end of the transaction. It is perfect for reentrancy locks and passing data between separate calls in a single transaction without paying storage costs.

```solidity
// Reentrancy lock using Transient Storage (saves ~2,000+ gas compared to SSTORE)
modifier nonReentrant() {
    assembly {
        if tload(0) { revert(0, 0) }
        tstore(0, 1)
    }
    _;
    assembly {
        tstore(0, 0)
    }
}
```

---

## 4. Overcoming the Stack Depth Limit (Stack Too Deep)

The EVM stack has a limit of 1024 slots, but instructions like `DUP` and `SWAP` can only reach the top **16 stack elements**. Attempting to access local variables deeper than 16 elements results in the infamous `"CompilerError: Stack too deep"` error.

### Strategies to Bypass Stack Limits:
1. **Use Block Scoping:** Introduce internal scopes `{}` to discard variables from the stack as soon as they are no longer needed.
2. **Use Structs or Arrays:** Grouping local parameters inside a struct or passing them as an array uses a single stack reference (a pointer to memory) instead of allocating separate stack slots for each member.
3. **Encapsulate Logic in Helper Functions:** Breaking long functions into modular, focused helper routines keeps the active stack frame clean.
