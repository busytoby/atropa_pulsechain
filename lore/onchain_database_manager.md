# On-Chain Database Manager (Solidity & Yul Hybrid)

This specification defines the architecture, storage layouts, and lookup schemas for a gas-optimized **On-Chain Database Manager** inspired by the 1980s **Mirage Concepts Database Manager** (incorporating free-form dynamic fields, custom calculated fields, and 16-bit indexing).

---

## 1. Storage Layout & Mirage Packing Schema

To support the free-form structure of the Mirage Database Manager (which allowed up to 200 fields per record and multi-level calculations) while minimizing gas on the EVM, the Database Manager implements **Dynamic Field Allocation** rather than static struct layouts.

### 1.1 Record Header Layout (256-bit Control Word)
Each record is indexed via a `uint16` identifier (supporting up to 65,535 records, matching Mirage's historical file limits). The first slot of a record acts as a Control Word:

| Field Name | Bit Range | Data Type | Description |
| :--- | :--- | :--- | :--- |
| **Record ID** | `[0..15]` | `uint16` | Unique record identifier (0–65535) |
| **Field Count** | `[16..23]` | `uint8` | Number of active fields in the record (max 200) |
| **Active Flags**| `[24..31]` | `uint8` | State flags (e.g. locked, synchronized) |
| **Field Offset Pointer** | `[32..255]` | `bytes28` | Packed offsets pointing to field text boundaries |

---

## 2. Dynamic Field Storage & Calculations

Following the Mirage Concepts design, fields can be dynamically recalculated.

### 2.1 Calculated Fields Engine
A dedicated Yul calculator allows field values to be computed on-the-fly using simple arithmetic formulas (e.g., `Field 3 = Field 1 * Field 2`).

```yul
// Calculates values dynamically for calculated fields in Yul
function evaluateCalculatedField(valA, valB, operator) -> result {
    switch operator
    case 1 { result := add(valA, valB) } // Addition
    case 2 { result := sub(valA, valB) } // Subtraction
    case 3 { result := mul(valA, valB) } // Multiplication
    case 4 {
        if valB { result := div(valA, valB) } // Division
    }
}
```

---

## 3. Hybrid Solidity-Yul Database Contract

The following blueprint demonstrates how Solidity manages interface definitions and access control, while inline Yul handles the raw bit-shifting for record packing and dynamic field lookups.

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

contract OnChainDatabaseManager {
    address public owner;
    
    // Base storage slot namespace for records
    bytes32 private constant RECORDS_SLOT_BASE = 0xa31e7d2b8b9a10242801b070e92313f0c5d5d121235B13a2B87B10242801b070;

    struct Record {
        uint16 recordId;
        uint8 fieldCount;
        uint8 flags;
        bytes32 fieldData;
    }

    modifier onlyOwner() {
        require(msg.sender == owner, "Not Authorized");
        _;
    }

    constructor() {
        owner = msg.sender;
    }

    /**
     * @notice Pack and insert a record directly into EVM storage using Yul.
     */
    function insertRecord(
        uint16 recordId,
        uint8 fieldCount,
        uint8 flags,
        bytes32 fieldData
    ) external onlyOwner {
        bytes32 targetSlot = getStorageSlot(recordId);

        assembly {
            // Pack variables into a single 256-bit control word
            let word := recordId
            word := or(word, shl(16, fieldCount))
            word := or(word, shl(24, flags))
            word := or(word, shl(32, shr(32, fieldData)))

            // Write to storage slot
            sstore(targetSlot, word)
        }
    }

    /**
     * @notice Retrieve and unpack a record directly using Yul.
     */
    function getRecord(uint16 recordId) external view returns (Record memory rec) {
        bytes32 targetSlot = getStorageSlot(recordId);

        assembly {
            let word := sload(targetSlot)
            
            // Unpack fields
            mstore(rec, and(word, 0xFFFF))                       // recordId
            mstore(add(rec, 32), and(shr(16, word), 0xFF))       // fieldCount
            mstore(add(rec, 64), and(shr(24, word), 0xFF))       // flags
            mstore(add(rec, 96), and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)) // fieldData
        }
    }

    /**
     * @notice Helper to calculate the record storage slot hash.
     */
    function getStorageSlot(uint16 recordId) public pure returns (bytes32) {
        return keccak256(abi.encodePacked(RECORDS_SLOT_BASE, recordId));
    }
}
```

---

## 4. Advantages of the Mirage-Inspired Hybrid Approach

1. **Free-Form Dynamic Fields**:
   By using packed offset pointers rather than rigid Solidity schemas, users can define and resize fields dynamically (up to 200 fields), mirroring the flexibility of the original Commodore 64 application.
2. **On-Chain Calculations**:
   The arithmetic helper engine enables instant on-chain field calculations, bypassing the need to compute results in client-side code.
3. **Optimized 16-Bit Indexing**:
   Matches the 65,535 file record boundary of the original software, keeping key-indexing words compact and gas consumption light.

