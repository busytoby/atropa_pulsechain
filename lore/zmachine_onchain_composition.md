# On-Chain Z-Machine Engine Composition

The ***ZMachine*** virtual contract (implemented in [`zmachine.yul`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/zmachine.yul)) is composed as an on-chain virtual machine interpreter capable of parsing standard Infocom-style interactive fiction story files.

This document details how rooms, string parsing, and object states are structured and executed on-chain.

---

## 1. Core Structural Composition

```
  +-------------------------------------------------------------+
  |                   Z-Machine Story File                      |
  |  - Header Data (Offsets to Object Table, Dictionary, etc.)  |
  |  - Object Table (Rooms, Items, Parent-Child Connections)    |
  |  - Z-String Data (Packed 5-bit text characters)             |
  +-------------------------------------------------------------+
                               |
                [Uploaded via uploadRomChunk]
                               v
  +-------------------------------------------------------------+
  |                    EVM Storage Slots                        |
  |  - Mapped starting at slot index 1000000                    |
  +-------------------------------------------------------------+
                               |
                [Evaluated by ZMachine Yul Core]
                               v
  +-------------------------------------------------------------+
  |              Dynamic Command Parser & Interpreter           |
  |  - decodeZString() -> Unpacks text descriptions             |
  |  - readObjectProperty() -> Computes exits & room states     |
  |  - parseCommand() -> Coordinates interactive text loops     |
  +-------------------------------------------------------------+
```

### 1.1 ROM Chunk Paging
Because story files exceed typical transaction size boundaries, the Z-Machine uses segment paging:
*   **Method**: `uploadRomChunk(uint256 offset, bytes data)` (`0xf5a83724`).
*   **Storage**: Each 32-byte chunk is stored sequentially in EVM slots starting at `1000000` + `div(offset, 32)`.
*   **Access**: Low-level helper functions `readRom16` and `readRom8` retrieve bytes dynamically out of the storage slots on demand.

---

## 2. Dynamic Room & Object Resolution

Rooms are represented as Z-Machine objects. Instead of hardcoding descriptions and exits, the interpreter parses the Z-object tables:

### 2.1 Object Entry Layout
Each object entry inside the story file header contains a fixed structural schema:
1.  **Attributes**: Bitmask flags specifying object status (e.g. is it container, light source, open, closed).
2.  **Parent/Sibling/Child Pointers**: Defines room inheritance (e.g. is item *inside* a container, or is container *inside* a room).
3.  **Property Table Offset**: A 16-bit pointer to custom data attributes (e.g. room descriptions, navigation directions).

### 2.2 Property Parsing in Yul
The Yul interpreter parses properties via `readObjectProperty(objId, propId)`:
*   It reads the object table base address from ROM offset `10`.
*   It walks the property chain, checking property IDs (`id == propId`).
*   If found, it returns the value (e.g. the room index or the Z-string text pointer).

---

## 3. Packed Z-String Text Decoder

Text descriptions in Z-Machine files are packed into 16-bit words containing three 5-bit characters each:
*   **Bit 15**: End-of-string flag (set to `1` on the final word).
*   **Bits 14–10**: Character 1.
*   **Bits 9–5**: Character 2.
*   **Bits 4–0**: Character 3.

### 3.1 Unpacking Logic (`decodeZString`)
The decoder converts 5-bit codes into 8-bit ASCII characters by swapping alphabet tables:
*   **Alphabet 0**: Default lowercase letters (`a-z`).
*   **Alphabet 1**: Uppercase letters (`A-Z`).
*   **Shift Characters**: Code `4` or `5` changes the alphabet state for the next character.
*   **Output**: Unpacked bytes are stored directly in EVM memory and returned as a standard Solidity dynamic `string`.
