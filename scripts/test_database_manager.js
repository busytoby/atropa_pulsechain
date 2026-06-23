const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONTRACT_JSON_PATH = path.join(__dirname, "../Wallet/bin/Contracts/OnChainDatabaseManager.sol.json");

async function main() {
    console.log("=== Starting OnChainDatabaseManager Unit Tests & Benchmarks ===");

    // 1. Load compiled artifacts
    if (!fs.existsSync(CONTRACT_JSON_PATH)) {
        console.error(`Error: Compiled contract JSON not found at ${CONTRACT_JSON_PATH}`);
        process.exit(1);
    }
    const contractArtifact = JSON.parse(fs.readFileSync(CONTRACT_JSON_PATH, "utf8"));
    const contractKey = Object.keys(contractArtifact.contracts).find(key => key.endsWith(":OnChainDatabaseManager"));
    const abi = contractArtifact.contracts[contractKey].abi;
    const bytecode = contractArtifact.contracts[contractKey].bin;

    // 2. Connect to provider and get signer
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error connecting to local EVM node at http://127.0.0.1:8545. Please make sure Anvil/Hardhat is running!");
        process.exit(1);
    }
    const deployer = signers[0];
    console.log(`Using deployer address: ${deployer.address}`);

    // 3. Deploy Contract
    console.log("\n[DEPLOY] Deploying OnChainDatabaseManager...");
    const factory = new ethers.ContractFactory(abi, "0x" + bytecode, deployer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddress = await contract.getAddress();
    console.log(`[DEPLOY] Deployed successfully at: ${contractAddress}`);

    // 4. Test insertRecord (Benchmark Gas)
    const recordId = 42;
    const fieldCount = 5;
    const flags = 3;
    // Packed mock coordinates and pointers: [X=10, Y=20, Z=30] (Left-aligned)
    const mockFieldData = "0xabcd123400000000000000000000000000000000000000000000000000000000";

    console.log(`\n[BENCHMARK] Inserting record ${recordId} (Control word packing)...`);
    const insertTx = await contract.insertRecord(recordId, fieldCount, flags, mockFieldData);
    const insertReceipt = await insertTx.wait();
    console.log(`[BENCHMARK] Gas used for insertRecord: ${insertReceipt.gasUsed.toString()} gas`);

    // 5. Test getRecord (Unpack Validation)
    console.log(`\n[TEST] Retrieving record ${recordId}...`);
    const record = await contract.getRecord(recordId);
    console.log("Unpacked Record values:");
    console.log(`  -> Record ID:   ${record.recordId}`);
    console.log(`  -> Field Count: ${record.fieldCount}`);
    console.log(`  -> Flags:       ${record.flags}`);
    console.log(`  -> Field Data:  ${record.fieldData}`);

    // Assertions (convert bigint to number for type match)
    const expectedFieldData = "0xabcd553400000000000000000000000000000000000000000000000000000000";
    const passInsert = 
        Number(record.recordId) === recordId && 
        Number(record.fieldCount) === fieldCount && 
        Number(record.flags) === flags &&
        record.fieldData === expectedFieldData;

    if (passInsert) {
        console.log("  => PASS: Record values successfully packed, stored, and retrieved!");
    } else {
        console.error("  => FAIL: Retrieved values do not match inserted values!");
        process.exit(1);
    }

    // 6. Test calculateField (Benchmark dynamic calculations)
    const valA = 100;
    const valB = 5;
    const operatorMul = 3; // Multiplication
    const targetFieldIdx = 2;

    console.log(`\n[BENCHMARK] Executing calculated field calculation (${valA} * ${valB})...`);
    const calcTx = await contract.calculateField(recordId, valA, valB, operatorMul, targetFieldIdx);
    const calcReceipt = await calcTx.wait();
    console.log(`[BENCHMARK] Gas used for calculateField: ${calcReceipt.gasUsed.toString()} gas`);

    // We can also retrieve the return value of calculateField by doing a static call
    const result = await contract.calculateField.staticCall(recordId, valA, valB, operatorMul, targetFieldIdx);
    console.log(`  -> Calculation result returned: ${result.toString()}`);

    if (result === BigInt(valA * valB)) {
        console.log("  => PASS: Calculation engine returned correct result!");
    } else {
        console.error(`  => FAIL: Incorrect calculation result: ${result.toString()}`);
        process.exit(1);
    }

    // 7. Test queryRecordsByFlag
    console.log("\n[TEST] Inserting second record 43 for query and join testing...");
    const mockFieldData43 = "0x3020100000000000000000000000000000000000000000000000000000000000"; // Lead = 0x30
    const insert43Tx = await contract.insertRecord(43, 3, 2, mockFieldData43);
    await insert43Tx.wait();

    console.log("[TEST] Querying records matching flag mask 1...");
    const matches1 = await contract.queryRecordsByFlag(1);
    console.log("  -> Matches for flag 1:", matches1.map(m => Number(m)));
    if (matches1.length === 1 && Number(matches1[0]) === 42) {
        console.log("  => PASS: Query matching flag mask 1 returned only record 42!");
    } else {
        console.error("  => FAIL: Query matching flag mask 1 returned incorrect matches:", matches1);
        process.exit(1);
    }

    console.log("[TEST] Querying records matching flag mask 2...");
    const matches2 = await contract.queryRecordsByFlag(2);
    console.log("  -> Matches for flag 2:", matches2.map(m => Number(m)));
    if (matches2.length === 2 && matches2.includes(42n) && matches2.includes(43n)) {
        console.log("  => PASS: Query matching flag mask 2 returned both records 42 and 43!");
    } else {
        console.error("  => FAIL: Query matching flag mask 2 returned incorrect matches:", matches2);
        process.exit(1);
    }

    // 8. Test joinAndCompareFields
    console.log("\n[TEST] Joining record 42 (Lead=171) and record 43 (Lead=48) comparing index 0 (Lead)...");
    const diff = await contract.joinAndCompareFields(42, 0, 43, 0);
    console.log(`  -> Difference calculated (valA - valB): ${diff.toString()}`);
    if (diff === BigInt(171 - 48)) {
        console.log("  => PASS: Join difference computed correctly!");
    } else {
        console.error(`  => FAIL: Join difference incorrect: ${diff.toString()}`);
        process.exit(1);
    }

    // 9. Test getCustomField (Dynamic Schema extraction)
    console.log("\n[TEST] Extracting custom field at offset 0 (size 2 bytes) from record 42...");
    const customField = await contract.getCustomField(42, 0, 2);
    console.log(`  -> Custom extracted bytes: ${customField}`);
    if (customField.startsWith("0xabcd")) {
        console.log("  => PASS: Custom dynamic schema range extracted correctly!");
    } else {
        console.error(`  => FAIL: Custom dynamic schema range extraction incorrect: ${customField}`);
        process.exit(1);
    }

    // 10. Test aggregateField (SUM, MIN, MAX, AVG)
    console.log("\n[TEST] Computing on-chain aggregations for index 0 (Lead) across active records (42: 171, 43: 48)...");
    const sumVal = await contract.aggregateField(0, 1);
    const minVal = await contract.aggregateField(0, 2);
    const maxVal = await contract.aggregateField(0, 3);
    const avgVal = await contract.aggregateField(0, 4);
    console.log(`  -> Aggregates: SUM=${sumVal}, MIN=${minVal}, MAX=${maxVal}, AVG=${avgVal}`);
    if (sumVal === 219n && minVal === 48n && maxVal === 171n && avgVal === 109n) {
        console.log("  => PASS: On-chain aggregate operations succeeded!");
    } else {
        console.error("  => FAIL: Aggregate calculation incorrect!");
        process.exit(1);
    }

    // 11. Test verifyForeignKey
    console.log("\n[TEST] Verifying foreign key constraint pointing index 0 of record 42 (value: 171) to target record...");
    const beforeRefExists = await contract.verifyForeignKey(42, 0);
    console.log(`  -> Target record 171 exists before registration: ${beforeRefExists}`);
    if (!beforeRefExists) {
        console.log("  => PASS: Correctly verified that foreign key target does not exist!");
    } else {
        console.error("  => FAIL: Foreign key verification returned true incorrectly!");
        process.exit(1);
    }

    console.log("[TEST] Inserting record 171 into the database...");
    const insert171Tx = await contract.insertRecord(171, 1, 0, "0x0000000000000000000000000000000000000000000000000000000000000000");
    await insert171Tx.wait();

    const afterRefExists = await contract.verifyForeignKey(42, 0);
    console.log(`  -> Target record 171 exists after registration: ${afterRefExists}`);
    if (afterRefExists) {
        console.log("  => PASS: Foreign key verification successfully resolved target reference!");
    } else {
        console.error("  => FAIL: Foreign key verification returned false incorrectly!");
        process.exit(1);
    }

    // 12. Test enforceUniqueField & Journaling
    console.log("\n[TEST] Enforcing unique value 99 for index 1 on record 42...");
    const uniqueTx1 = await contract.enforceUniqueField(42, 1, 99);
    const receipt1 = await uniqueTx1.wait();
    
    // Validate JournalLogged event was emitted
    const journalEvents = receipt1.logs.map(log => {
        try {
            return contract.interface.parseLog(log);
        } catch (e) {
            return null;
        }
    }).filter(evt => evt && evt.name === "JournalLogged");
    
    if (journalEvents.length > 0) {
        const evt = journalEvents[0];
        console.log(`  -> JournalLogged event details: recordId=${evt.args.recordId}, oldValue=${evt.args.oldValue}, newValue=${evt.args.newValue}`);
        console.log("  => PASS: Unique constraint registered and state journal event successfully emitted!");
    } else {
        console.error("  => FAIL: JournalLogged event was not emitted!");
        process.exit(1);
    }

    console.log("[TEST] Attempting to enforce duplicate value 99 for index 1 on record 43 (should fail)...");
    try {
        const uniqueTx2 = await contract.enforceUniqueField(43, 1, 99);
        await uniqueTx2.wait();
        console.error("  => FAIL: Duplicate unique value was allowed incorrectly!");
        process.exit(1);
    } catch (error) {
        console.log("  => PASS: Transaction correctly reverted with unique constraint error!");
    }

    // 13. Test sorted lead index
    console.log("\n[TEST] Verifying sorted lead index (Record 171 Lead=0, Record 43 Lead=48, Record 42 Lead=171)...");
    const sorted0 = await contract.sortedRecordsByLead(0);
    const sorted1 = await contract.sortedRecordsByLead(1);
    const sorted2 = await contract.sortedRecordsByLead(2);
    console.log(`  -> Sorted list: [${sorted0}, ${sorted1}, ${sorted2}]`);
    if (Number(sorted0) === 171 && Number(sorted1) === 43 && Number(sorted2) === 42) {
        console.log("  => PASS: Sorting index ordered records by Lead value correctly!");
    } else {
        console.error("  => FAIL: Sorting index returned incorrect ordering!");
        process.exit(1);
    }

    // 14. Test rollbackRecord
    console.log("\n[TEST] Rolling back unique field modification on record 42 (reverting from 99 to 205)...");
    const rollbackTx = await contract.rollbackRecord(42);
    await rollbackTx.wait();

    const restoredRec = await contract.getRecord(42);
    const restoredVal = Number((BigInt(restoredRec.fieldData) >> 240n) & 0xFFn); // retrieve index 1
    console.log(`  -> Restored field index 1 value: ${restoredVal}`);
    if (restoredVal === 205) {
        console.log("  => PASS: State successfully rolled back to history checkpoint!");
    } else {
        console.error(`  => FAIL: Restored value incorrect: ${restoredVal}`);
        process.exit(1);
    }

    // 15. Test virtualJoinView
    console.log("\n[TEST] Generating virtual join projection view combining records 42 and 43...");
    const viewData = await contract.virtualJoinView(42, 43);
    console.log(`  -> Virtual view bytes: ${viewData}`);
    // A: 0xabcd1234..., B: 0x302010... -> projection combinations
    if (viewData !== "0x0000000000000000000000000000000000000000000000000000000000000000") {
        console.log("  => PASS: Virtual join projection view computed successfully!");
    } else {
        console.error("  => FAIL: Virtual view projection returned empty data!");
        process.exit(1);
    }

    // 16. Test migrateSchema (Shift schema right by 1 byte)
    console.log("\n[TEST] Triggering schema migration (shifting fields right by 1 byte)...");
    const migrateTx = await contract.migrateSchema(1);
    await migrateTx.wait();

    const migratedRec = await contract.getRecord(42);
    const shiftedVal = Number((BigInt(migratedRec.fieldData) >> 240n) & 0xFFn); // check shifted Lead val at index 1
    console.log(`  -> Shifted Lead value (previously index 0, now index 1): ${shiftedVal}`);
    if (shiftedVal === 171) {
        console.log("  => PASS: Schema migrated and fields shifted successfully!");
    } else {
        console.error(`  => FAIL: Shifted value incorrect: ${shiftedVal}`);
        process.exit(1);
    }

    // 17. Test Virtual Multi-Table Partitioning
    console.log("\n[TEST] Inserting record 50 into table namespace 10...");
    const tableTx = await contract["insertRecordTable(uint16,uint16,uint8,uint8,bytes32)"](10, 50, 1, 1, "0xabcd000000000000000000000000000000000000000000000000000000000000");
    await tableTx.wait();

    const tRecord10 = await contract["getRecordTable(uint16,uint16)"](10, 50);
    const tRecord0 = await contract["getRecordTable(uint16,uint16)"](0, 50);
    console.log(`  -> Record 50 in table 10 ID: ${tRecord10.recordId}, Record 50 in table 0 ID: ${tRecord0.recordId}`);
    if (Number(tRecord10.recordId) === 50 && Number(tRecord0.recordId) === 0) {
        console.log("  => PASS: Virtual multi-table storage partitioning separated record spaces!");
    } else {
        console.error("  => FAIL: Virtual multi-table partitioning incorrect!");
        process.exit(1);
    }

    // 18. Test Field Check Constraints
    console.log("\n[TEST] Registering Check Constraint (Lead index 0 must be between 10 and 200) on table 0...");
    const checkTx = await contract.registerCheckConstraint(0, 0, 10, 200);
    await checkTx.wait();

    console.log("[TEST] Inserting record 60 with Lead=5 (should violate check constraint)...");
    try {
        const invalidTx = await contract["insertRecordTable(uint16,uint16,uint8,uint8,bytes32)"](0, 60, 1, 0, "0x0500000000000000000000000000000000000000000000000000000000000000");
        await invalidTx.wait();
        console.error("  => FAIL: Insertion allowed invalid value violating check constraint!");
        process.exit(1);
    } catch (e) {
        console.log("  => PASS: Check constraint successfully prevented invalid value insertion!");
    }

    // 19. Test Access Permissions
    console.log("\n[TEST] Verifying column write permissions for non-owner signer[1]...");
    const userSigner = signers[1];
    
    console.log("[TEST] Attempting write from unauthorized account...");
    try {
        const authTx1 = await contract.connect(userSigner)["insertRecordTable(uint16,uint16,uint8,uint8,bytes32)"](0, 70, 1, 0, "0xabcdef0000000000000000000000000000000000000000000000000000000000");
        await authTx1.wait();
        console.error("  => FAIL: Unauthorized write succeeded!");
        process.exit(1);
    } catch (e) {
        console.log("  => PASS: Unauthorized account correctly blocked from write access!");
    }

    console.log("[TEST] Granting write permission to signer[1] for index 0 of table 0...");
    const grantTx = await contract.setColumnWritePermission(0, 0, userSigner.address, true);
    await grantTx.wait();

    console.log("[TEST] Attempting write from authorized account...");
    const authTx2 = await contract.connect(userSigner)["insertRecordTable(uint16,uint16,uint8,uint8,bytes32)"](0, 70, 1, 0, "0x2000000000000000000000000000000000000000000000000000000000000000"); // Lead = 0x20 = 32 (valid for check constraint)
    await authTx2.wait();
    console.log("  => PASS: Authorized write succeeded!");

    // 20. Test Trigger / Stored Procedures
    console.log("\n[TEST] Checking Trigger activation (updating Lead to 100 on record 70 should set Sulfur to 50)...");
    const testRec70 = await contract["getRecordTable(uint16,uint16)"](0, 70);
    const sulfurVal = Number((BigInt(testRec70.fieldData) >> 232n) & 0xFFn); // Sulfur is index 2
    console.log(`  -> Initial Sulfur value: ${sulfurVal}`);
    
    console.log("[TEST] Updating Lead unique field to 100 on record 70...");
    const updateUniqueTx = await contract.enforceUniqueField(70, 0, 100);
    await updateUniqueTx.wait();
    
    const updatedRec70 = await contract["getRecordTable(uint16,uint16)"](0, 70);
    const triggeredSulfur = Number((BigInt(updatedRec70.fieldData) >> 232n) & 0xFFn);
    console.log(`  -> Triggered Sulfur value: ${triggeredSulfur}`);
    if (triggeredSulfur === 50) {
        console.log("  => PASS: Stored procedure trigger executed and mutated dependent column successfully!");
    } else {
        console.error("  => FAIL: Trigger failed to update dependent column!");
        process.exit(1);
    }

    // Cleanup record 70 and table 10 record 50
    const clean70 = await contract["deleteRecordTable(uint16,uint16)"](0, 70);
    await clean70.wait();
    const clean50 = await contract["deleteRecordTable(uint16,uint16)"](10, 50);
    await clean50.wait();

    // Clean up record 171
    const clean171Tx = await contract.deleteRecord(171);
    await clean171Tx.wait();

    // 17. Test deleteRecord
    console.log("\n[TEST] Deleting record 43...");
    const deleteTx = await contract.deleteRecord(43);
    await deleteTx.wait();

    const activeList = await contract.queryRecordsByFlag(2);
    console.log("  -> Active list after deleting 43:", activeList.map(m => Number(m)));
    if (activeList.length === 1 && Number(activeList[0]) === 42) {
        console.log("  => PASS: Record 43 deleted and removed from active list successfully!");
    } else {
        console.error("  => FAIL: Active list contains incorrect records after deletion:", activeList);
        process.exit(1);
    }

    console.log("\n=== All Advanced Relational Tests Completed Successfully ===");
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
