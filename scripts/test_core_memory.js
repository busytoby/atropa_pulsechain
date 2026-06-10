const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Coincident Current Ferrite Core Memory Emulator (core_memory.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/core_memory.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/core_memory.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const coreABI = [
        "function writeCore(uint256 x, uint256 y, uint8 bit) external returns (uint256)",
        "function readCoreDestructive(uint256 x, uint256 y) external returns (uint8 bit, uint256 sensePulse)",
        "function readRestoreCore(uint256 x, uint256 y) external returns (uint8 bit)"
    ];

    const factory = new ethers.ContractFactory(coreABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`CoreMemory contract deployed at: ${contractAddr}\n`);

    const gasLimit = 150000;

    // 1. Write core states
    console.log("Writing 1 at (3, 4)...");
    await (await contract.writeCore(3, 4, 1, { gasLimit })).wait();

    console.log("Writing 1 at (5, 6)...");
    await (await contract.writeCore(5, 6, 1, { gasLimit })).wait();

    console.log("Writing 0 at (7, 8)...");
    await (await contract.writeCore(7, 8, 0, { gasLimit })).wait();

    // 2. Test Destructive Read
    console.log("\nReading (3, 4) destructively...");
    let read34 = await contract.readCoreDestructive.staticCall(3, 4);
    console.log(`Read returns -> bit: ${read34.bit}, sensePulse: ${read34.sensePulse}`);
    if (Number(read34.bit) !== 1 || Number(read34.sensePulse) !== 1) {
        throw new Error("Failed to read core state '1' destructively!");
    }
    await (await contract.readCoreDestructive(3, 4, { gasLimit })).wait();

    // Confirm core (3, 4) state is now 0 (destroyed)
    let reRead34 = await contract.readCoreDestructive.staticCall(3, 4);
    console.log(`Re-reading (3, 4) destructively -> bit: ${reRead34.bit}, sensePulse: ${reRead34.sensePulse}`);
    if (Number(reRead34.bit) !== 0 || Number(reRead34.sensePulse) !== 0) {
        throw new Error("Destructive read did not clear the core state!");
    }
    console.log("Destructive read test: Passed!\n");

    // 3. Test Read-Restore Cycle
    console.log("Reading & Restoring (5, 6)...");
    let read56 = await contract.readRestoreCore.staticCall(5, 6);
    console.log(`Read-Restore returns -> bit: ${read56}`);
    if (Number(read56) !== 1) {
        throw new Error("Failed to read core state '1' during restore cycle!");
    }
    await (await contract.readRestoreCore(5, 6, { gasLimit })).wait();

    // Confirm state is STILL 1 (restored)
    let reRead56 = await contract.readRestoreCore.staticCall(5, 6);
    console.log(`Re-reading (5, 6) -> bit: ${reRead56} (Expected: 1)`);
    if (Number(reRead56) !== 1) {
        throw new Error("Read-Restore cycle failed to rewrite the state!");
    }
    console.log("Read-Restore cycle test: Passed!\n");

    // 4. Test reading 0
    console.log("Reading (7, 8)...");
    let read78 = await contract.readRestoreCore.staticCall(7, 8);
    console.log(`Read (7, 8) returns -> bit: ${read78}`);
    if (Number(read78) !== 0) {
        throw new Error("Mismatched reading for 0 state core!");
    }
    console.log("Read-Restore 0 state test: Passed!\n");

    console.log(">>> ALL FERRITE CORE MEMORY TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
