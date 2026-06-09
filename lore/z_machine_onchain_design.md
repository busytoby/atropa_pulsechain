# On-Chain Z-Machine (Version 3) Design

We can implement a Z-machine (Version 3) interpreter directly in Yul. Version 3 (Z3) is the classic "Standard" interpreter used by *Zork I-III*, *Planetfall*, and *The Hitchhiker's Guide to the Galaxy*. It is extremely space-efficient, fitting inside small memory bounds.

---

## 1. Z3 Memory Map Layout (On-Chain Slots)

The Z-machine uses a byte-addressable flat memory space. In Yul, we can store this inside dynamic storage slots namespaced to the user:

```
+------------------------------------------+ 0x0000
| Header (64 bytes)                        |
| - Version (byte 0 = 3)                   |
| - High byte of High Memory (byte 4)      |
| - Initial Program Counter (bytes 6-7)   |
| - Dictionary address (bytes 8-9)         |
| - Object table address (bytes 10-11)     |
| - Global variables address (bytes 12-13) |
+------------------------------------------+
| Dynamic Memory (Read/Write)              |
| - Objects, Globals, Game State           |
+------------------------------------------+ Base of Static Memory
| Static Memory (Read-Only)                |
| - Dictionary, Grammar tables             |
+------------------------------------------+ Base of High Memory
| High Memory (Instructions & Strings)     |
+------------------------------------------+ End of Story File
```

---

## 2. On-Chain Storage Mapping
To prevent storage collisions between players or games, we map the Z-machine RAM to namespaced storage slots using our existing pattern:
$$\text{StorageSlot} = \text{keccak256}(\text{userAddress} \parallel \text{gameId} \parallel \text{wordIndex})$$

Each storage slot holds a standard EVM 32-byte word (representing 32 bytes of Z-machine memory).

---

## 3. Z3 Opcode Categories to Implement
To run a classic Z3 story file, we must support:
*   **2OP (Two-Operand Instructions)**: `je` (jump if equal), `jl` (jump if less than), `jg`, `dec_chk`, `inc_chk`, `test`, `or`, `and`, `test_attr`, `set_attr`, `clear_attr`, `store`, `insert_obj`, `loadw`, `loadb`, `get_prop`, `get_prop_addr`, `get_next_prop`, `add`, `sub`, `mul`, `div`, `mod`.
*   **1OP (One-Operand Instructions)**: `jz` (jump if zero), `get_sibling`, `get_child`, `get_parent`, `get_prop_len`, `inc`, `dec`, `print_addr`, `remove_obj`, `print_obj`, `ret`, `jump`, `print_paddr`, `load`, `not`.
*   **0OP (Zero-Operand Instructions)**: `rtrue` (return true), `rfalse` (return false), `print` (print inline literal string), `print_ret` (print literal string and return true), `nop`, `save`, `restore`, `restart`, `ret_popped`, `pop`, `quit`, `new_line`, `show_status`, `verify`.
*   **VAR (Variable-Operand Instructions)**: `call`, `storew`, `storeb`, `put_prop`, `sread` (read input line), `print_char`, `print_num`, `random`, `push`, `pull`, `split_window`, `set_window`.

---

## 4. Text Engine (ZSCII & Z-characters)
Z-machine text is heavily compressed:
*   Every character string is composed of **5-bit Z-characters** packed three to a 16-bit word.
*   Uses three alphabets (A0, A1, A2) shifted dynamically using shift codes `4` and `5`.
*   Supports abbreviation tables to reduce text size (perfect for on-chain gas optimization).

---

## Next Steps
1. **Develop `zinterpreter.yul`**: Draft a clean Yul contract implementing the Z3 main execution loop, instruction fetch, decode, and stack operations.
2. **Write Z3 Test Script**: Create a script loading a tiny Z3 test story file to verify instruction execution.
