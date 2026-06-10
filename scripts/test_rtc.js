const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Real-Time Clock (RTC) Memory Registers ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling and deploying folklore.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/folklore.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const folkABI = [
        "function peek(uint256 addr) external view returns (uint256)"
    ];

    const factory = new ethers.ContractFactory(folkABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Folklore CPU deployed at: ${contractAddr}`);

    // Fetch block timestamp directly to compare
    const block = await provider.getBlock("latest");
    const blockTime = block.timestamp;

    // Read RTC registers:
    // 55600: Seconds
    // 55601: Minutes
    // 55602: Hours
    // 55603: Full Epoch Timestamp
    const rtcSec = await contract.peek(55600);
    const rtcMin = await contract.peek(55601);
    const rtcHour = await contract.peek(55602);
    const rtcTime = await contract.peek(55603);

    console.log(`\nLatest block timestamp: ${blockTime}`);
    console.log(`RTC Epoch Timestamp:    ${rtcTime}`);
    console.log(`RTC Hour Register:      ${rtcHour}`);
    console.log(`RTC Minute Register:    ${rtcMin}`);
    console.log(`RTC Second Register:    ${rtcSec}`);

    // Verify
    const expectedSec = (blockTime % 3600) % 60;
    const expectedMin = Math.floor((blockTime % 3600) / 60);
    const expectedHour = Math.floor((blockTime % 86400) / 3600);

    const match = Number(rtcTime) === blockTime && 
                  Number(rtcSec) === expectedSec && 
                  Number(rtcMin) === expectedMin && 
                  Number(rtcHour) === expectedHour;

    if (match) {
        console.log("\n>>> SUCCESS: Real-Time Clock memory register calculations verified perfectly! <<<");
    } else {
        console.error("\n>>> FAILURE: RTC calculations mismatched! <<<");
        process.exit(1);
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
