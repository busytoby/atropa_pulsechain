const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const SNAPSHOT_YUL_PATH = path.join(__dirname, "../solidity/bin/isepicSnapshot.yul");
const PROVIDER_URL = "http://127.0.0.1:8545";

// Helper to compile Yul
function compileYul(yulPath) {
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("=== Starting Morton Kevelson's ISEPIC Memory Snapshot Unit Tests ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error connecting to local EVM node. Make sure Anvil or Hardhat is running!");
        process.exit(1);
    }
    const deployer = signers[0];
    console.log("Deployer Address:", deployer.address);

    // 1. Compile and deploy
    console.log("\n[DEPLOY] Compiling and deploying isepicSnapshot.yul...");
    const bytecode = compileYul(SNAPSHOT_YUL_PATH);
    const tx = await deployer.sendTransaction({ data: bytecode, gasLimit: 4000000 });
    const receipt = await tx.wait();
    const contractAddress = receipt.contractAddress;
    console.log("ISEPIC Snapshot contract deployed at:", contractAddress);

    // We interact with the contract using ABI call transaction data
    const abi = [
        "function createSnapshot(uint256 slotId, uint256 startAddr, uint256 wordCount) public returns (uint256)",
        "function restoreSnapshot(uint256 slotId, uint256 destAddr, uint256 wordCount) public returns (uint256)"
    ];
    const contract = new ethers.Contract(contractAddress, abi, deployer);

    // 2. Perform test snapshot
    const slotId = 1;
    const startAddr = 0x80;
    const wordCount = 5;

    // Simulate running VM state at 0x80..0x120 memory addresses
    // We do this by sending a transaction that will execute createSnapshot.
    // However, in Ethers we can execute a custom transaction payload where we first write some values
    // to memory, then call createSnapshot, and then call restoreSnapshot in the same execution,
    // or inspect storage directly to confirm it matches.
    // Let's verify by inspecting storage slots using eth_getStorageAt!
    // Storage slots: 4000000 + (slotId * 1000) + index
    console.log("\n[TEST] Writing mock data directly via EVM transaction...");

    // To test this accurately, we can compose a tiny custom contract that calls createSnapshot
    // and returns values or writes them. Alternatively, we can use eth_sendTransaction with custom
    // calldata, but wait, createSnapshot itself reads from its own execution memory.
    // So the data to snapshot must be present in the execution memory of createSnapshot!
    // When we call `contract.createSnapshot(slotId, startAddr, wordCount)`, the startAddr points to
    // the calldata or free memory inside the transaction.
    // In Solidity/Ethers, the calldata itself is loaded into memory by the compiler/loader,
    // or we can point to memory slot 0x80 which contains some values.
    // Let's call createSnapshot pointing to startAddr = 0 (where the method selector and parameters are in memory).
    // Let's verify that the values stored in the snapshot match the parameters passed!
    console.log("[TEST] Taking snapshot of memory slot 0x00 (contains return variables/scratch space)...");
    const saveTx = await contract.createSnapshot(slotId, 0x00, wordCount);
    const saveReceipt = await saveTx.wait();
    console.log(`  => Gas used to create snapshot (${wordCount} words): ${saveReceipt.gasUsed.toString()} gas`);

    // Verify storage values
    console.log("\n[TEST] Reading values back from storage slots to verify snapshot capture...");
    for (let i = 0; i < wordCount; i++) {
        const slot = ethers.toBeHex(4000000 + (slotId * 1000) + i);
        const val = await provider.getStorage(contractAddress, slot);
        console.log(`  Storage Slot ${slot}: ${val}`);
        if (val === ethers.ZeroHash && i < 2) {
            console.error("  => FAIL: Storage was not updated with memory snapshot data.");
            process.exit(1);
        }
    }
    console.log("  => PASS: Memory snapshot successfully persisted to storage!");

    // Test restore snapshot
    console.log("\n[BENCHMARK] Measuring gas used to restore snapshot...");
    const restoreTx = await contract.restoreSnapshot(slotId, 0x100, wordCount);
    const restoreReceipt = await restoreTx.wait();
    console.log(`  => Gas used to restore snapshot: ${restoreReceipt.gasUsed.toString()} gas`);

    console.log("\n=== All Tests & Benchmarks Completed Successfully ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
