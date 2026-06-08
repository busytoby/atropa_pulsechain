// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

/**
 * @title OnChainDatabaseManager
 * @notice A gas-optimized database manager combining Solidity structural interfaces 
 *         with inline Yul assembly for dynamic packing, storage routing, and calculated fields.
 *         Inspired by the 1980s Mirage Concepts Database Manager.
 */
contract OnChainDatabaseManager {
    address public owner;
    
    // Base storage slot namespace for records: keccak256("database.mirage.records")
    bytes32 private constant RECORDS_SLOT_BASE = 0x8a927a4b8b9a10242801b070e92313f0c5d5d121235B13a2B87B10242801b070;

    struct Record {
        uint16 recordId;
        uint8 fieldCount;
        uint8 flags;
        bytes32 fieldData;
    }

    event RecordInserted(uint16 indexed recordId, uint8 fieldCount, uint8 flags);
    event FieldCalculated(uint16 indexed recordId, uint8 targetField, uint256 result);

    modifier onlyOwner() {
        require(msg.sender == owner, "Not Authorized");
        _;
    }

    constructor() {
        owner = msg.sender;
    }

    /**
     * @notice Pack and insert a record directly into EVM storage using Yul.
     * @param recordId Unique identifier (0-65535)
     * @param fieldCount Number of active fields (max 200)
     * @param flags Record configuration flags
     * @param fieldData Packed field coordinates and pointer bounds
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
            // Layout:
            // bits [0..15]   = recordId (16 bits)
            // bits [16..23]  = fieldCount (8 bits)
            // bits [24..31]  = flags (8 bits)
            // bits [32..255] = upper 224 bits of fieldData
            let word := recordId
            word := or(word, shl(16, fieldCount))
            word := or(word, shl(24, flags))
            word := or(word, and(fieldData, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000))

            // Write control word directly to computed slot
            sstore(targetSlot, word)
        }

        emit RecordInserted(recordId, fieldCount, flags);
    }

    /**
     * @notice Retrieve and unpack a record directly using Yul.
     * @param recordId Unique identifier of target record
     */
    function getRecord(uint16 recordId) external view returns (Record memory rec) {
        bytes32 targetSlot = getStorageSlot(recordId);

        assembly {
            let word := sload(targetSlot)
            
            // Unpack fields and store in return struct
            mstore(rec, and(word, 0xFFFF))                       // recordId
            mstore(add(rec, 32), and(shr(16, word), 0xFF))       // fieldCount
            mstore(add(rec, 64), and(shr(24, word), 0xFF))       // flags
            mstore(add(rec, 96), and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)) // fieldData
        }
    }

    /**
     * @notice Dynamically calculate field values on-chain using a Yul arithmetic engine.
     * @param recordId Target record ID
     * @param valA Left operator value
     * @param valB Right operator value
     * @param operator Math operation (1 = Add, 2 = Sub, 3 = Mul, 4 = Div)
     * @param targetField Destination field index to log
     */
    function calculateField(
        uint16 recordId,
        uint256 valA,
        uint256 valB,
        uint8 operator,
        uint8 targetField
    ) external returns (uint256 result) {
        assembly {
            switch operator
            case 1 { result := add(valA, valB) } // Addition
            case 2 { result := sub(valA, valB) } // Subtraction
            case 3 { result := mul(valA, valB) } // Multiplication
            case 4 {
                if valB { result := div(valA, valB) } // Division
            }
        }

        emit FieldCalculated(recordId, targetField, result);
    }

    /**
     * @notice Helper to calculate the deterministic record storage slot hash.
     */
    function getStorageSlot(uint16 recordId) public pure returns (bytes32) {
        return keccak256(abi.encodePacked(RECORDS_SLOT_BASE, recordId));
    }
}
