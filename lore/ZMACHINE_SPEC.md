# Z-Machine Emulator Technical Specification

This document serves as the absolute technical reference for the Z-Machine virtual environment running on EVM storage.

---

## 1. Storage Layout & Memory Partitioning
The Z-Machine uses custom segmented storage slots to partition ROM, game state, items, and player details:

| Address Segment Offset | Usage | Storage Slot Structure |
|---|---|---|
| `1000000 + offset` | **Game ROM Header & Code** | Uploaded via chunked transactions. |
| `2000000 + objId` | **Contract Bindings** | Mapped ERC20 / QING token contract addresses. |
| `2000300 + itemId` | **Item Placements** | Current Room ID location of the item (Room `0` is Inventory). |
| `2500000` | **Folklore Address** | Pointer to the Gauntlet / Folklore CPU contract. |
| `2600000` | **Parser Address** | Pointer to the active Z-Machine Parser contract. |
| `3000000 + roomId` | **Custom Room Data Length** | Dynamic description string length. |
| `3100000 + (roomId * 100) + word` | **Room Custom Text (DNA)** | Packed ASCII character words. |
| `3200000 + roomId` | **Room Exits Map** | Packed byte offsets of adjacent rooms. |
| `4000000 + actorIdentity` | **Actor Room Locations** | Current Room ID of a Player (address) or NPC (card ID `0` to `5`). |
| `5100000 + roomId` | **Enemy Health** | Hit points of dynamic monsters in the room. |
| `5200000 + roomId` | **Enemy Dead State** | Boolean death flags (`1` = defeated, `0` = alive). |

---

## 2. Room Exits Encoding (`3200000 + roomId`)
A single 32-byte storage word encodes adjacent exits using the first 4 bytes of the slot:
* **Byte 0**: North Room ID
* **Byte 1**: South Room ID
* **Byte 2**: East Room ID
* **Byte 3**: West Room ID

*Example*: If slot `3200010` contains `0x0B0C0E00`, the room has exits to Room `11` (North), Room `12` (South), and Room `14` (East).

---

## 3. Parser Vocabulary & Verbs
The parser matches the first word of command input (bytes shifted right by 224 bits):

| Verb | Hex Signature | Action |
|---|---|---|
| **look** | `0x6c6f6f6b` | Appends room description, items, other actors, and bound QING/VOID contract commands. |
| **north** / **south** / **east** / **west** | Shift / move logic | Moves Player/NPC between rooms by writing to slot `4000000 + actorIdentity`. |
| **take** | `0x74616b65` | Picks up item in the room, transferring it to Inventory (Room `0`) and triggers ERC20 reward. |
| **use [item]** | `0x75736520` | Swipes items (like Keycard) to unlock portal arches (e.g. opens North exit). |
| **play [timbre]** | `0x706c6179` | Modulates the synthesizers and outputs current room frequency/notes. |
| **attack** | `0x61747461` | Deals damage to the active room monster using equipped Gauntlet weapons. |
| **Chat [msg]** | `0x43686174` | Submits message logs to bound on-chain QING/VOID chat contracts. |

---

## 4. NPC Integration & Encounter Details
* **Process Cards**: AI NPCs run inside WinchesterMQ scheduler execution slices, mapped as Process Cards `0` to `5`.
* **Encounter Trigger**: When Room IDs match between Player address `A` and Card ID `C` (e.g. `sload(4000000 + A) == sload(4000000 + C)`), the parser prints:
  `[NPC] Active Process CardC  standing here.`
