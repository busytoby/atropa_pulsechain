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
    const contractKey = Object.keys(contractArtifact.contracts)[0];
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
    const passInsert = 
        Number(record.recordId) === recordId && 
        Number(record.fieldCount) === fieldCount && 
        Number(record.flags) === flags &&
        record.fieldData === mockFieldData;

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

    console.log("\n=== All Tests & Benchmarks Completed Successfully ===");
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
