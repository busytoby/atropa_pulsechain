// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

interface IAlchemicalGold {
    function mint(address to, uint256 value) external;
    function burn(address from, uint256 value) external;
}

interface IRandomGenerator {
    function getRandomValues() external returns (bytes32[] memory);
}

/**
 * @title OnChainDatabaseManager
 * @notice A gas-optimized database manager combining Solidity structural interfaces 
 *         with inline Yul assembly for dynamic packing, storage routing, and calculated fields.
 *         Inspired by the 1980s Mirage Concepts Database Manager.
 */
contract OnChainDatabaseManager {
    address public owner;
    address public goldToken;
    address public randomGenerator;
    
    // Dysnomia routing
    address public systemRegistry;
    address public diyatContract;
    
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
    event MutationTriggered(uint16 indexed recordId, uint8 indexed targetIndex, uint256 oldVal, uint256 newVal);

    modifier onlyOwner() {
        require(msg.sender == owner, "Not Authorized");
        _;
    }

    constructor() {
        owner = msg.sender;
    }

    function setGoldToken(address _goldToken) external onlyOwner {
        goldToken = _goldToken;
    }

    function setRandomGenerator(address _randomGenerator) external onlyOwner {
        randomGenerator = _randomGenerator;
    }

    function setSystemRegistry(address _registry) external onlyOwner {
        systemRegistry = _registry;
    }

    function setDiyatContract(address _diyat) external onlyOwner {
        diyatContract = _diyat;
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
     * @notice Perform a fully on-chain alchemical transmutation and update state directly in storage.
     * @param recordId Target player record ID
     * @param indexA First reagent index in bytes32 fieldData
     * @param indexB Second reagent index in bytes32 fieldData
     * @param operator Math operator (1 = Add, 2 = Sub, 3 = Mul, 4 = Div)
     * @param targetIndex Destination reagent index to update in bytes32 fieldData
     */
    function transmute(
        uint16 recordId,
        uint8 indexA,
        uint8 indexB,
        uint8 operator,
        uint8 targetIndex
    ) external {
        bytes32 targetSlot = getStorageSlot(recordId);
        uint256 oldTargetVal;
        uint256 newTargetVal;
        assembly {
            let word := sload(targetSlot)
            
            // Extract packed fieldData (upper 224 bits)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            
            // Retrieve valA and valB from the bytes32 fieldData structure
            // Offsets: index * 8 bits (1 byte per reagent)
            // Left-aligned in bytes32: offset is 248 - (index * 8)
            let offsetA := sub(248, mul(indexA, 8))
            let valA := and(shr(offsetA, fieldData), 0xFF)
            
            let offsetB := sub(248, mul(indexB, 8))
            let valB := and(shr(offsetB, fieldData), 0xFF)
 
            // Save old value of target reagent
            let targetOffset := sub(248, mul(targetIndex, 8))
            oldTargetVal := and(shr(targetOffset, fieldData), 0xFF)
            
            // Compute calculation via dynamic operator
            let result := 0
            switch operator
            case 1 { result := add(valA, valB) } // Addition
            case 2 { result := sub(valA, valB) } // Subtraction
            case 3 { result := mul(valA, valB) } // Multiplication
            case 4 {
                if valB { result := div(valA, valB) } // Division
            }
            
            // Clamp result to 1-byte limits (0-255)
            if gt(result, 255) { result := 255 }
            newTargetVal := result
            
            // Clear old target value in fieldData and pack new result
            let mask := not(shl(targetOffset, 0xFF))
            fieldData := and(fieldData, mask)
            fieldData := or(fieldData, shl(targetOffset, and(result, 0xFF)))
            
            // Re-compile control word and write back
            let newWord := and(word, 0xFFFFFFFF) // Keep recordId, fieldCount, flags
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }
 
        emit FieldCalculated(recordId, targetIndex, newTargetVal); // Trigger event logs
 
        // Dysnomia Random Generator Catalyst Mutation
        if (randomGenerator != address(0)) {
            try IRandomGenerator(randomGenerator).getRandomValues() returns (bytes32[] memory randomValues) {
                if (randomValues.length > 0) {
                    uint256 randIdx = block.number % randomValues.length;
                    uint256 seed = uint256(randomValues[randIdx]);
                    
                    // Modulo check for random outcome (0-99)
                    uint256 rolled = seed % 100;
                    uint256 mutatedVal = newTargetVal;
                    
                    if (rolled < 10) {
                        // Critical success: Double the output value!
                        mutatedVal = newTargetVal * 2;
                        if (mutatedVal > 255) mutatedVal = 255;
                        emit MutationTriggered(recordId, targetIndex, newTargetVal, mutatedVal);
                    } else if (rolled > 90) {
                        // Dissociation hazard: Cut value in half!
                        mutatedVal = newTargetVal / 2;
                        emit MutationTriggered(recordId, targetIndex, newTargetVal, mutatedVal);
                    }
                    
                    if (mutatedVal != newTargetVal) {
                        newTargetVal = mutatedVal;
                        // Save mutated value back to storage using assembly
                        assembly {
                            let word := sload(targetSlot)
                            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
                            let targetOffset := sub(248, mul(targetIndex, 8))
                            let mask := not(shl(targetOffset, 0xFF))
                            fieldData := and(fieldData, mask)
                            fieldData := or(fieldData, shl(targetOffset, and(mutatedVal, 0xFF)))
                            let newWord := and(word, 0xFFFFFFFF)
                            newWord := or(newWord, fieldData)
                            sstore(targetSlot, newWord)
                        }
                    }
                }
            } catch {}
        }

        // If targetIndex is 4 (Gold) and goldToken is configured
        if (targetIndex == 4 && goldToken != address(0)) {
            if (newTargetVal > oldTargetVal) {
                uint256 diff = newTargetVal - oldTargetVal;
                IAlchemicalGold(goldToken).mint(msg.sender, diff * 10**18);
            } else if (newTargetVal < oldTargetVal) {
                uint256 diff = oldTargetVal - newTargetVal;
                IAlchemicalGold(goldToken).burn(msg.sender, diff * 10**18);
            }
        }
    }

    mapping(uint16 => uint256) public lastDecayBlock;

    event SubstrateDecayed(uint16 indexed recordId, bytes32 newFieldData);

    /**
     * @notice Performs alchemical decay on a packed player substrate record.
     *         Decays elements over blocks: Lead decays to Mercury, Sulfur burns if Salt is low.
     *         Can be triggered by anyone if block.number has increased.
     */
    function executeDecay(uint16 recordId) external returns (bytes32 newFieldData) {
        // Prevent multiple executions in the same block
        require(block.number > lastDecayBlock[recordId], "Already decayed in this block");
        
        // Initialize last decay block if first run
        if (lastDecayBlock[recordId] == 0) {
            lastDecayBlock[recordId] = block.number;
            Record memory rec = this.getRecord(recordId);
            return rec.fieldData;
        }

        uint256 elapsedBlocks = block.number - lastDecayBlock[recordId];
        lastDecayBlock[recordId] = block.number;

        bytes32 targetSlot = getStorageSlot(recordId);
        uint256 oldGold;
        uint256 newGold;

        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)

            // Offsets: Lead = 0 (248), Mercury = 1 (240), Sulfur = 2 (232), Salt = 3 (224), Gold = 4 (216)
            let lead := and(shr(248, fieldData), 0xFF)
            let mercury := and(shr(240, fieldData), 0xFF)
            let sulfur := and(shr(232, fieldData), 0xFF)
            let salt := and(shr(224, fieldData), 0xFF)
            oldGold := and(shr(216, fieldData), 0xFF)

            // Lead Decay: Lead decays into Mercury slowly (1 unit per 20 elapsed blocks)
            let leadDecayAmt := div(elapsedBlocks, 20)
            if gt(leadDecayAmt, 0) {
                if gt(leadDecayAmt, lead) { leadDecayAmt := lead }
                lead := sub(lead, leadDecayAmt)
                mercury := add(mercury, leadDecayAmt)
                if gt(mercury, 255) { mercury := 255 }
            }

            // Sulfur Instability: If Sulfur is greater than twice the Salt amount, Sulfur combusts,
            // destroying Sulfur and Gold.
            let saltThreshold := mul(salt, 2)
            if gt(sulfur, saltThreshold) {
                let combustionAmt := div(elapsedBlocks, 10)
                if gt(combustionAmt, 0) {
                    // Reduce sulfur
                    if gt(combustionAmt, sulfur) { combustionAmt := sulfur }
                    sulfur := sub(sulfur, combustionAmt)

                    // Damage Gold due to combustion shock wave
                    let goldDamage := div(combustionAmt, 2)
                    if gt(goldDamage, oldGold) { goldDamage := oldGold }
                    newGold := sub(oldGold, goldDamage)
                }
            }
            if eq(newGold, 0) {
                newGold := oldGold
            }

            // Re-pack new state values
            fieldData := and(fieldData, 0x0000000000000000000000000000000000000000000000000000000000000000) // Clear
            fieldData := or(fieldData, shl(248, lead))
            fieldData := or(fieldData, shl(240, mercury))
            fieldData := or(fieldData, shl(232, sulfur))
            fieldData := or(fieldData, shl(224, salt))
            fieldData := or(fieldData, shl(216, newGold))

            // Re-compile control word and write back
            let newWord := and(word, 0xFFFFFFFF) // Keep recordId, fieldCount, flags
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
            newFieldData := fieldData
        }

        // Handle burn event if Gold decreased during combustion decay
        if (newGold < oldGold && goldToken != address(0)) {
            uint256 diff = oldGold - newGold;
            IAlchemicalGold(goldToken).burn(msg.sender, diff * 10**18);
        }

        emit SubstrateDecayed(recordId, newFieldData);
    }

    event SaltPurchased(uint16 indexed recordId, uint8 saltAmount);

    /**
     * @notice Purchase Salt stabilizer by burning Alchemical Gold (Au) ERC-20 tokens.
     *         Price: 10 Au per 1 unit of Salt.
     */
    function buySalt(uint16 recordId, uint8 saltAmount) external {
        require(goldToken != address(0), "Gold token not bound");
        require(saltAmount > 0, "Amount must be positive");

        uint256 cost = uint256(saltAmount) * 10 * 10**18;
        
        // Burn Au tokens from user's wallet
        IAlchemicalGold(goldToken).burn(msg.sender, cost);

        bytes32 targetSlot = getStorageSlot(recordId);
        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)

            // Salt offset is 3 (224 bits)
            let salt := and(shr(224, fieldData), 0xFF)
            salt := add(salt, saltAmount)
            if gt(salt, 255) { salt := 255 }

            // Clear old salt value (offset 224 bits, mask: clear 8 bits at 224)
            let mask := not(shl(224, 0xFF))
            fieldData := and(fieldData, mask)
            
            // Re-pack new salt
            fieldData := or(fieldData, shl(224, and(salt, 0xFF)))

            // Re-compile control word and write back
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }

        emit SaltPurchased(recordId, saltAmount);
    }

    /**
     * @notice Helper to calculate the deterministic record storage slot hash.
     */
    function getStorageSlot(uint16 recordId) public pure returns (bytes32) {
        return keccak256(abi.encodePacked(RECORDS_SLOT_BASE, recordId));
    }
}
