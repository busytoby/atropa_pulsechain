# Integrating ERC-20 Tokens with the Z-Machine Object Table

To make Zork-like games that interact natively with on-chain assets, we can map ERC-20 token balances directly to objects in the Z-machine's Object Table.

---

## 1. Z-Machine Object Table Architecture

In a standard Z3 story file, the Object Table defines all "things", "rooms", and "actors". Each object has:
1.  **Attributes** (32 boolean flags representing state, like `is_takeable`, `is_lit`, `is_worn`).
2.  **Parent, Sibling, and Child** pointers (establishing room/inventory hierarchies).
3.  **Properties** (custom variables like description, weight, or value).

```
+---------------------------------------------+
| Object ID (e.g. 50: "Gold Coin Token")      |
| - Parent: Player's Inventory (Object ID 1)   |
| - Attributes: Takeable, Value-bearing       |
| - Custom Property (0x20): ERC-20 Address    |
+---------------------------------------------+
```

---

## 2. On-Chain Binding (ERC-20 <-> Objects)

Instead of hardcoding static quantities inside the story file, we intercept properties or attributes of specific Object IDs:

*   **Mapping Object Properties to Contract Calls**:
    If an object has a custom property (e.g., `TokenContractAddress`), the interpreter retrieves the player's balance dynamically from that ERC-20 contract instead of reading it from static memory.
*   **Triggering On-Chain Mints/Transfers**:
    When the Z-machine processes actions like:
    *   `take gold coin` -> Triggers an ERC-20 `transfer` or `mint` to the player's address.
    *   `drop gold coin` -> Triggers an ERC-20 `transfer` or `burn` from the player's address.

---

## 3. Implementation Workflow

1.  **Develop ZSCII Decoding Engine**: Write Yul functions to decompress 5-bit Z-characters into standard 8-bit UTF-8 strings.
2.  **Extend `zmachine.yul`**: Build the object property accessor functions, adding conditionals that execute dynamic `staticcall` or `call` operations to external ERC-20 addresses when querying specific object values.
