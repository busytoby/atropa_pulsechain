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

    uint16[] public recordList;
    uint16[] public sortedRecordsByLead;

    mapping(uint16 => uint16[]) public tableRecordList;
    mapping(uint16 => mapping(uint16 => uint256)) private listIndex; // tableId => recordId => index + 1
    mapping(bytes32 => bool) private uniqueFields; // Unique key constraint mapping: keccak256(tableId, index, value)

    struct StateLog {
        uint8 index;
        uint8 value;
    }
    mapping(uint16 => mapping(uint16 => StateLog[])) private stateHistory; // tableId => recordId => Log
    mapping(uint16 => uint16[]) public tableSortedRecordsByLead; // tableId => List

    struct CheckConstraint {
        uint8 index;
        uint8 minValue;
        uint8 maxValue;
        bool active;
    }
    mapping(uint16 => mapping(uint8 => CheckConstraint)) public tableCheckConstraints; // tableId => index => Constraint
    mapping(uint16 => mapping(uint8 => mapping(address => bool))) public columnWritePermissions; // tableId => index => user => allowed

    event RecordInserted(uint16 indexed recordId, uint8 fieldCount, uint8 flags);
    event RecordDeleted(uint16 indexed recordId);
    event FieldCalculated(uint16 indexed recordId, uint8 targetField, uint256 result);
    event MutationTriggered(uint16 indexed recordId, uint8 indexed targetIndex, uint256 oldVal, uint256 newVal);
    event JournalLogged(uint16 indexed recordId, uint8 indexed index, uint256 oldValue, uint256 newValue, uint256 blockNumber);

    event TableRecordInserted(uint16 indexed tableId, uint16 indexed recordId, uint8 fieldCount, uint8 flags);
    event TableRecordDeleted(uint16 indexed tableId, uint16 indexed recordId);
    event TableFieldCalculated(uint16 indexed tableId, uint16 indexed recordId, uint8 targetField, uint256 result);
    event TableJournalLogged(uint16 indexed tableId, uint16 indexed recordId, uint8 indexed index, uint256 oldValue, uint256 newValue, uint256 blockNumber);
    event TriggerExecuted(uint16 indexed tableId, uint16 indexed recordId, uint8 triggeredIndex, uint256 value);
    event SaltPurchased(uint16 indexed recordId, uint8 saltAmount);
    event SubstrateDecayed(uint16 indexed recordId, bytes32 newFieldData);

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

    function registerCheckConstraint(uint16 tableId, uint8 index, uint8 minVal, uint8 maxVal) external onlyOwner {
        tableCheckConstraints[tableId][index] = CheckConstraint(index, minVal, maxVal, true);
    }

    function setColumnWritePermission(uint16 tableId, uint8 index, address user, bool allowed) external onlyOwner {
        columnWritePermissions[tableId][index][user] = allowed;
    }

    function validateCheckConstraint(uint16 tableId, uint8 index, uint8 value) public view {
        CheckConstraint memory c = tableCheckConstraints[tableId][index];
        if (c.active) {
            require(value >= c.minValue && value <= c.maxValue, "Check constraint violated");
        }
    }

    function verifyWritePermission(uint16 tableId, uint8 index, address user) public view {
        if (user != owner) {
            require(columnWritePermissions[tableId][index][user], "Write permission denied");
        }
    }

    function executeTriggers(uint16 tableId, uint16 recordId, uint8 index, uint8 value) internal {
        if (index == 0) { // Lead update trigger -> auto-update Sulfur (index 2) to value / 2
            uint8 autoSulfur = value / 2;
            bytes32 targetSlot = getStorageSlot(tableId, recordId);
            assembly {
                let word := sload(targetSlot)
                let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
                let targetOffset := sub(248, mul(2, 8)) // index 2 (Sulfur)
                let mask := not(shl(targetOffset, 0xFF))
                fieldData := and(fieldData, mask)
                fieldData := or(fieldData, shl(targetOffset, and(autoSulfur, 0xFF)))
                let newWord := and(word, 0xFFFFFFFF)
                newWord := or(newWord, fieldData)
                sstore(targetSlot, newWord)
            }
            emit TriggerExecuted(tableId, recordId, 2, autoSulfur);
        }
    }

    function getStorageSlot(uint16 tableId, uint16 recordId) public pure returns (bytes32) {
        return keccak256(abi.encodePacked(RECORDS_SLOT_BASE, tableId, recordId));
    }

    function getStorageSlot(uint16 recordId) public pure returns (bytes32) {
        return getStorageSlot(0, recordId);
    }

    function insertRecordTable(
        uint16 tableId,
        uint16 recordId,
        uint8 fieldCount,
        uint8 flags,
        bytes32 fieldData
    ) public {
        verifyWritePermission(tableId, 0, msg.sender);
        uint8 leadVal = uint8(uint256(fieldData >> 248) & 0xFF);
        validateCheckConstraint(tableId, 0, leadVal);

        bytes32 targetSlot = getStorageSlot(tableId, recordId);
        assembly {
            let word := recordId
            word := or(word, shl(16, fieldCount))
            word := or(word, shl(24, flags))
            word := or(word, and(fieldData, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000))
            sstore(targetSlot, word)
        }

        if (listIndex[tableId][recordId] == 0) {
            tableRecordList[tableId].push(recordId);
            listIndex[tableId][recordId] = tableRecordList[tableId].length;
            if (tableId == 0) {
                recordList.push(recordId);
            }
        }

        updateSortedLeadIndex(tableId, recordId, leadVal);
        executeTriggers(tableId, recordId, 0, leadVal);

        if (tableId == 0) {
            emit RecordInserted(recordId, fieldCount, flags);
        } else {
            emit TableRecordInserted(tableId, recordId, fieldCount, flags);
        }
    }

    function insertRecord(
        uint16 recordId,
        uint8 fieldCount,
        uint8 flags,
        bytes32 fieldData
    ) external onlyOwner {
        insertRecordTable(0, recordId, fieldCount, flags, fieldData);
    }

    function getRecordTable(uint16 tableId, uint16 recordId) public view returns (Record memory rec) {
        bytes32 targetSlot = getStorageSlot(tableId, recordId);
        assembly {
            let word := sload(targetSlot)
            mstore(rec, and(word, 0xFFFF))
            mstore(add(rec, 32), and(shr(16, word), 0xFF))
            mstore(add(rec, 64), and(shr(24, word), 0xFF))
            mstore(add(rec, 96), and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000))
        }
    }

    function getRecord(uint16 recordId) external view returns (Record memory rec) {
        return getRecordTable(0, recordId);
    }

    function calculateField(
        uint16 recordId,
        uint256 valA,
        uint256 valB,
        uint8 operator,
        uint8 targetField
    ) external returns (uint256 result) {
        assembly {
            switch operator
            case 1 { result := add(valA, valB) }
            case 2 { result := sub(valA, valB) }
            case 3 { result := mul(valA, valB) }
            case 4 {
                if valB { result := div(valA, valB) }
            }
        }
        emit FieldCalculated(recordId, targetField, result);
    }

    function transmute(
        uint16 recordId,
        uint8 indexA,
        uint8 indexB,
        uint8 operator,
        uint8 targetIndex
    ) external {
        bytes32 targetSlot = getStorageSlot(0, recordId);
        uint256 oldTargetVal;
        uint256 newTargetVal;
        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            let offsetA := sub(248, mul(indexA, 8))
            let valA := and(shr(offsetA, fieldData), 0xFF)
            let offsetB := sub(248, mul(indexB, 8))
            let valB := and(shr(offsetB, fieldData), 0xFF)
            let targetOffset := sub(248, mul(targetIndex, 8))
            oldTargetVal := and(shr(targetOffset, fieldData), 0xFF)
            
            let result := 0
            switch operator
            case 1 { result := add(valA, valB) }
            case 2 { result := sub(valA, valB) }
            case 3 { result := mul(valA, valB) }
            case 4 {
                if valB { result := div(valA, valB) }
            }
            if gt(result, 255) { result := 255 }
            newTargetVal := result
            
            let mask := not(shl(targetOffset, 0xFF))
            fieldData := and(fieldData, mask)
            fieldData := or(fieldData, shl(targetOffset, and(result, 0xFF)))
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }
        emit FieldCalculated(recordId, targetIndex, newTargetVal);

        if (randomGenerator != address(0)) {
            try IRandomGenerator(randomGenerator).getRandomValues() returns (bytes32[] memory randomValues) {
                if (randomValues.length > 0) {
                    uint256 randIdx = block.number % randomValues.length;
                    uint256 seed = uint256(randomValues[randIdx]);
                    uint256 rolled = seed % 100;
                    uint256 mutatedVal = newTargetVal;
                    if (rolled < 10) {
                        mutatedVal = newTargetVal * 2;
                        if (mutatedVal > 255) mutatedVal = 255;
                        emit MutationTriggered(recordId, targetIndex, newTargetVal, mutatedVal);
                    } else if (rolled > 90) {
                        mutatedVal = newTargetVal / 2;
                        emit MutationTriggered(recordId, targetIndex, newTargetVal, mutatedVal);
                    }
                    if (mutatedVal != newTargetVal) {
                        newTargetVal = mutatedVal;
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
        if (targetIndex == 4 && goldToken != address(0)) {
            if (newTargetVal > oldTargetVal) {
                IAlchemicalGold(goldToken).mint(msg.sender, (newTargetVal - oldTargetVal) * 10**18);
            } else if (newTargetVal < oldTargetVal) {
                IAlchemicalGold(goldToken).burn(msg.sender, (oldTargetVal - newTargetVal) * 10**18);
            }
        }
    }

    mapping(uint16 => uint256) public lastDecayBlock;

    function executeDecay(uint16 recordId) external returns (bytes32 newFieldData) {
        require(block.number > lastDecayBlock[recordId], "Already decayed in this block");
        if (lastDecayBlock[recordId] == 0) {
            lastDecayBlock[recordId] = block.number;
            Record memory rec = this.getRecord(recordId);
            return rec.fieldData;
        }
        uint256 elapsedBlocks = block.number - lastDecayBlock[recordId];
        lastDecayBlock[recordId] = block.number;
        bytes32 targetSlot = getStorageSlot(0, recordId);
        uint256 oldGold;
        uint256 newGold;
        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            let lead := and(shr(248, fieldData), 0xFF)
            let mercury := and(shr(240, fieldData), 0xFF)
            let sulfur := and(shr(232, fieldData), 0xFF)
            let salt := and(shr(224, fieldData), 0xFF)
            oldGold := and(shr(216, fieldData), 0xFF)
            let leadDecayAmt := div(elapsedBlocks, 20)
            if gt(leadDecayAmt, 0) {
                if gt(leadDecayAmt, lead) { leadDecayAmt := lead }
                lead := sub(lead, leadDecayAmt)
                mercury := add(mercury, leadDecayAmt)
                if gt(mercury, 255) { mercury := 255 }
            }
            let saltThreshold := mul(salt, 2)
            if gt(sulfur, saltThreshold) {
                let combustionAmt := div(elapsedBlocks, 10)
                if gt(combustionAmt, 0) {
                    if gt(combustionAmt, sulfur) { combustionAmt := sulfur }
                    sulfur := sub(sulfur, combustionAmt)
                    let goldDamage := div(combustionAmt, 2)
                    if gt(goldDamage, oldGold) { goldDamage := oldGold }
                    newGold := sub(oldGold, goldDamage)
                }
            }
            if eq(newGold, 0) { newGold := oldGold }
            fieldData := and(fieldData, 0x00)
            fieldData := or(fieldData, shl(248, lead))
            fieldData := or(fieldData, shl(240, mercury))
            fieldData := or(fieldData, shl(232, sulfur))
            fieldData := or(fieldData, shl(224, salt))
            fieldData := or(fieldData, shl(216, newGold))
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
            newFieldData := fieldData
        }
        if (newGold < oldGold && goldToken != address(0)) {
            IAlchemicalGold(goldToken).burn(msg.sender, (oldGold - newGold) * 10**18);
        }
        emit SubstrateDecayed(recordId, newFieldData);
    }

    function buySalt(uint16 recordId, uint8 saltAmount) external {
        require(goldToken != address(0), "Gold token not bound");
        require(saltAmount > 0, "Amount must be positive");
        IAlchemicalGold(goldToken).burn(msg.sender, uint256(saltAmount) * 10 * 10**18);
        bytes32 targetSlot = getStorageSlot(0, recordId);
        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            let salt := and(shr(224, fieldData), 0xFF)
            salt := add(salt, saltAmount)
            if gt(salt, 255) { salt := 255 }
            let mask := not(shl(224, 0xFF))
            fieldData := and(fieldData, mask)
            fieldData := or(fieldData, shl(224, and(salt, 0xFF)))
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }
        emit SaltPurchased(recordId, saltAmount);
    }

    function deleteRecordTable(uint16 tableId, uint16 recordId) public {
        verifyWritePermission(tableId, 0, msg.sender);
        bytes32 targetSlot = getStorageSlot(tableId, recordId);
        assembly {
            sstore(targetSlot, 0)
        }

        uint256 idxPlusOne = listIndex[tableId][recordId];
        if (idxPlusOne > 0) {
            uint256 index = idxPlusOne - 1;
            uint256 lastIndex = tableRecordList[tableId].length - 1;
            if (index != lastIndex) {
                uint16 lastRecordId = tableRecordList[tableId][lastIndex];
                tableRecordList[tableId][index] = lastRecordId;
                listIndex[tableId][lastRecordId] = index + 1;
            }
            tableRecordList[tableId].pop();
            delete listIndex[tableId][recordId];

            if (tableId == 0) {
                // Sync legacy recordList
                int256 found = -1;
                for (uint256 i = 0; i < recordList.length; i++) {
                    if (recordList[i] == recordId) {
                        found = int256(i);
                        break;
                    }
                }
                if (found >= 0) {
                    uint256 lIdx = uint256(found);
                    recordList[lIdx] = recordList[recordList.length - 1];
                    recordList.pop();
                }
            }
        }

        // Remove from sorted list
        int256 foundIdx = -1;
        for (uint256 i = 0; i < tableSortedRecordsByLead[tableId].length; i++) {
            if (tableSortedRecordsByLead[tableId][i] == recordId) {
                foundIdx = int256(i);
                break;
            }
        }
        if (foundIdx >= 0) {
            uint256 idx = uint256(foundIdx);
            for (uint256 i = idx; i < tableSortedRecordsByLead[tableId].length - 1; i++) {
                tableSortedRecordsByLead[tableId][i] = tableSortedRecordsByLead[tableId][i+1];
            }
            tableSortedRecordsByLead[tableId].pop();

            if (tableId == 0) {
                // Legacy sorted sync
                int256 fIdx = -1;
                for (uint256 i = 0; i < sortedRecordsByLead.length; i++) {
                    if (sortedRecordsByLead[i] == recordId) {
                        fIdx = int256(i);
                        break;
                    }
                }
                if (fIdx >= 0) {
                    uint256 idxL = uint256(fIdx);
                    for (uint256 i = idxL; i < sortedRecordsByLead.length - 1; i++) {
                        sortedRecordsByLead[i] = sortedRecordsByLead[i+1];
                    }
                    sortedRecordsByLead.pop();
                }
            }
        }

        if (tableId == 0) {
            emit RecordDeleted(recordId);
        } else {
            emit TableRecordDeleted(tableId, recordId);
        }
    }

    function deleteRecord(uint16 recordId) external onlyOwner {
        deleteRecordTable(0, recordId);
    }

    function queryRecordsByFlagTable(uint16 tableId, uint8 flagMask) public view returns (uint16[] memory matches) {
        uint256 count = 0;
        uint16[] memory list = tableRecordList[tableId];
        for (uint256 i = 0; i < list.length; i++) {
            Record memory rec = getRecordTable(tableId, list[i]);
            if ((rec.flags & flagMask) == flagMask) {
                count++;
            }
        }
        matches = new uint16[](count);
        uint256 idx = 0;
        for (uint256 i = 0; i < list.length; i++) {
            Record memory rec = getRecordTable(tableId, list[i]);
            if ((rec.flags & flagMask) == flagMask) {
                matches[idx] = list[i];
                idx++;
            }
        }
    }

    function queryRecordsByFlag(uint8 flagMask) external view returns (uint16[] memory matches) {
        return queryRecordsByFlagTable(0, flagMask);
    }

    function joinAndCompareFieldsTable(
        uint16 tableIdA,
        uint16 recordIdA,
        uint8 indexA,
        uint16 tableIdB,
        uint16 recordIdB,
        uint8 indexB
    ) public view returns (int256 diff) {
        Record memory recA = getRecordTable(tableIdA, recordIdA);
        Record memory recB = getRecordTable(tableIdB, recordIdB);
        uint256 valA;
        uint256 valB;
        assembly {
            let dataA := mload(add(recA, 96))
            let offsetA := sub(248, mul(indexA, 8))
            valA := and(shr(offsetA, dataA), 0xFF)
            let dataB := mload(add(recB, 96))
            let offsetB := sub(248, mul(indexB, 8))
            valB := and(shr(offsetB, dataB), 0xFF)
        }
        diff = int256(valA) - int256(valB);
    }

    function joinAndCompareFields(
        uint16 recordIdA,
        uint8 indexA,
        uint16 recordIdB,
        uint8 indexB
    ) external view returns (int256 diff) {
        return joinAndCompareFieldsTable(0, recordIdA, indexA, 0, recordIdB, indexB);
    }

    function getCustomFieldTable(
        uint16 tableId,
        uint16 recordId,
        uint8 offsetBytes,
        uint8 sizeBytes
    ) public view returns (bytes32 result) {
        require(sizeBytes > 0 && sizeBytes <= 32, "Invalid size");
        Record memory rec = getRecordTable(tableId, recordId);
        bytes32 fieldData = rec.fieldData;
        assembly {
            let bitsOffset := mul(offsetBytes, 8)
            let bitsSize := mul(sizeBytes, 8)
            let temp := shl(bitsOffset, fieldData)
            let mask := not(shr(bitsSize, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
            result := and(temp, mask)
        }
    }

    function getCustomField(
        uint16 recordId,
        uint8 offsetBytes,
        uint8 sizeBytes
    ) external view returns (bytes32 result) {
        return getCustomFieldTable(0, recordId, offsetBytes, sizeBytes);
    }

    function aggregateFieldTable(uint16 tableId, uint8 index, uint8 op) public view returns (uint256 result) {
        uint16[] memory list = tableRecordList[tableId];
        if (list.length == 0) return 0;
        uint256 count = list.length;
        uint256 sum = 0;
        uint256 min = type(uint256).max;
        uint256 max = 0;
        for (uint256 i = 0; i < count; i++) {
            Record memory rec = getRecordTable(tableId, list[i]);
            uint256 val;
            assembly {
                let data := mload(add(rec, 96))
                let offset := sub(248, mul(index, 8))
                val := and(shr(offset, data), 0xFF)
            }
            sum += val;
            if (val < min) min = val;
            if (val > max) max = val;
        }
        if (op == 1) result = sum;
        else if (op == 2) result = min;
        else if (op == 3) result = max;
        else if (op == 4) result = sum / count;
    }

    function aggregateField(uint8 index, uint8 op) external view returns (uint256 result) {
        return aggregateFieldTable(0, index, op);
    }

    function verifyForeignKeyTable(uint16 tableIdA, uint16 recordIdA, uint8 indexA) public view returns (bool) {
        Record memory rec = getRecordTable(tableIdA, recordIdA);
        uint256 val;
        assembly {
            let data := mload(add(rec, 96))
            let offset := sub(248, mul(indexA, 8))
            val := and(shr(offset, data), 0xFF)
        }
        uint16 targetRecordId = uint16(val);
        return listIndex[tableIdA][targetRecordId] > 0;
    }

    function verifyForeignKey(uint16 recordIdA, uint8 indexA) external view returns (bool) {
        return verifyForeignKeyTable(0, recordIdA, indexA);
    }

    function enforceUniqueFieldTable(uint16 tableId, uint16 recordId, uint8 index, uint8 value) public {
        verifyWritePermission(tableId, index, msg.sender);
        validateCheckConstraint(tableId, index, value);

        bytes32 hashKey = keccak256(abi.encodePacked(tableId, index, value));
        require(!uniqueFields[hashKey], "Unique constraint violated");

        Record memory rec = getRecordTable(tableId, recordId);
        uint8 oldVal;
        assembly {
            let data := mload(add(rec, 96))
            let offset := sub(248, mul(index, 8))
            oldVal := and(shr(offset, data), 0xFF)
        }
        bytes32 oldHashKey = keccak256(abi.encodePacked(tableId, index, oldVal));
        delete uniqueFields[oldHashKey];

        stateHistory[tableId][recordId].push(StateLog(index, oldVal));

        if (index == 0) {
            updateSortedLeadIndex(tableId, recordId, value);
        }

        bytes32 targetSlot = getStorageSlot(tableId, recordId);
        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            let targetOffset := sub(248, mul(index, 8))
            let mask := not(shl(targetOffset, 0xFF))
            fieldData := and(fieldData, mask)
            fieldData := or(fieldData, shl(targetOffset, and(value, 0xFF)))
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }

        uniqueFields[hashKey] = true;
        executeTriggers(tableId, recordId, index, value);

        if (tableId == 0) {
            emit JournalLogged(recordId, index, oldVal, value, block.number);
        } else {
            emit TableJournalLogged(tableId, recordId, index, oldVal, value, block.number);
        }
    }

    function enforceUniqueField(uint16 recordId, uint8 index, uint8 value) external onlyOwner {
        enforceUniqueFieldTable(0, recordId, index, value);
    }

    function rollbackRecordTable(uint16 tableId, uint16 recordId) public {
        verifyWritePermission(tableId, 0, msg.sender);
        require(stateHistory[tableId][recordId].length > 0, "No state history found");
        uint256 lastIdx = stateHistory[tableId][recordId].length - 1;
        StateLog memory log = stateHistory[tableId][recordId][lastIdx];
        stateHistory[tableId][recordId].pop();

        bytes32 targetSlot = getStorageSlot(tableId, recordId);
        uint8 oldVal;
        uint8 index = log.index;
        uint8 value = log.value;

        assembly {
            let word := sload(targetSlot)
            let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
            let targetOffset := sub(248, mul(index, 8))
            oldVal := and(shr(targetOffset, fieldData), 0xFF)
            let mask := not(shl(targetOffset, 0xFF))
            fieldData := and(fieldData, mask)
            fieldData := or(fieldData, shl(targetOffset, and(value, 0xFF)))
            let newWord := and(word, 0xFFFFFFFF)
            newWord := or(newWord, fieldData)
            sstore(targetSlot, newWord)
        }

        bytes32 oldHashKey = keccak256(abi.encodePacked(tableId, index, oldVal));
        delete uniqueFields[oldHashKey];
        bytes32 restoredHashKey = keccak256(abi.encodePacked(tableId, index, value));
        uniqueFields[restoredHashKey] = true;

        if (index == 0) {
            updateSortedLeadIndex(tableId, recordId, value);
        }

        if (tableId == 0) {
            emit JournalLogged(recordId, index, oldVal, value, block.number);
        } else {
            emit TableJournalLogged(tableId, recordId, index, oldVal, value, block.number);
        }
    }

    function rollbackRecord(uint16 recordId) external onlyOwner {
        rollbackRecordTable(0, recordId);
    }

    function migrateSchemaTable(uint16 tableId, uint8 shiftBytes) public onlyOwner {
        uint16[] memory list = tableRecordList[tableId];
        for (uint256 i = 0; i < list.length; i++) {
            uint16 rid = list[i];
            bytes32 targetSlot = getStorageSlot(tableId, rid);
            assembly {
                let word := sload(targetSlot)
                let fieldData := and(word, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000)
                let shiftedData := shr(mul(shiftBytes, 8), fieldData)
                let newWord := and(word, 0xFFFFFFFF)
                newWord := or(newWord, and(shiftedData, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000))
                sstore(targetSlot, newWord)
            }
        }
    }

    function migrateSchema(uint8 shiftBytes) external onlyOwner {
        migrateSchemaTable(0, shiftBytes);
    }

    function virtualJoinViewTable(
        uint16 tableIdA,
        uint16 recordIdA,
        uint16 tableIdB,
        uint16 recordIdB
    ) public view returns (bytes32 viewData) {
        Record memory recA = getRecordTable(tableIdA, recordIdA);
        Record memory recB = getRecordTable(tableIdB, recordIdB);
        assembly {
            let dataA := mload(add(recA, 96))
            let dataB := mload(add(recB, 96))
            viewData := or(
                and(dataA, 0xFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000000000),
                shr(128, and(dataB, 0xFFFFFFFFFFFFFFFF000000000000000000000000000000000000000000000000))
            )
        }
    }

    function virtualJoinView(uint16 recordIdA, uint16 recordIdB) external view returns (bytes32 viewData) {
        return virtualJoinViewTable(0, recordIdA, 0, recordIdB);
    }

    function updateSortedLeadIndex(uint16 tableId, uint16 recordId, uint8 leadValue) internal {
        uint16[] storage sList = tableSortedRecordsByLead[tableId];
        int256 foundIdx = -1;
        for (uint256 i = 0; i < sList.length; i++) {
            if (sList[i] == recordId) {
                foundIdx = int256(i);
                break;
            }
        }
        if (foundIdx >= 0) {
            uint256 idx = uint256(foundIdx);
            for (uint256 i = idx; i < sList.length - 1; i++) {
                sList[i] = sList[i+1];
            }
            sList.pop();
        }
        uint256 insertIdx = sList.length;
        for (uint256 i = 0; i < sList.length; i++) {
            Record memory rec = getRecordTable(tableId, sList[i]);
            uint8 val;
            assembly {
                let data := mload(add(rec, 96))
                val := and(shr(248, data), 0xFF)
            }
            if (leadValue < val) {
                insertIdx = i;
                break;
            }
        }
        sList.push(0);
        for (uint256 i = sList.length - 1; i > insertIdx; i--) {
            sList[i] = sList[i-1];
        }
        sList[insertIdx] = recordId;

        // Legacy sync
        if (tableId == 0) {
            while (sortedRecordsByLead.length > 0) {
                sortedRecordsByLead.pop();
            }
            for (uint256 i = 0; i < sList.length; i++) {
                sortedRecordsByLead.push(sList[i]);
            }
        }
    }
}
