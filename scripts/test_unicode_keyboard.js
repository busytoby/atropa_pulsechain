const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Unicode Keyboard FIFO Buffer on Folklore CPU ===");

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
        "function pushUnicodeKey(uint256 codePoint) external returns (uint256)",
        "function popUnicodeKey() external returns (uint256)",
        "function peek(uint256 addr) external view returns (uint256)"
    ];

    const factory = new ethers.ContractFactory(folkABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Folklore CPU contract deployed at: ${contractAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");
    async function sendTx(method, ...args) {
        const tx = await contract[method](...args, { nonce });
        nonce++;
        return tx.wait();
    }

    // Unicode test sequence: "H", "e", "l", "l", "o", " ", "👋", "🚀"
    // "H" = 72, "e" = 101, "l" = 108, "o" = 111, "👋" = 0x1F44B (128075), "🚀" = 0x1F680 (128640)
    const testKeys = [72, 101, 108, 108, 111, 32, 128075, 128640];

    console.log("\nSimulating rapid keystroke / paste clipboard inputs...");
    for (const key of testKeys) {
        await sendTx("pushUnicodeKey", key);
        console.log(` -> Pushed code point: ${key} (0x${key.toString(16).toUpperCase()})`);
    }

    console.log("\nPopping inputs from Folklore CPU FIFO buffer...");
    let poppedKeys = [];
    for (let i = 0; i < testKeys.length; i++) {
        // Read value first using staticCall
        const key = await contract.popUnicodeKey.staticCall();
        poppedKeys.push(Number(key));
        console.log(` -> Popped code point: ${key} (${String.fromCodePoint(Number(key))})`);
        
        // Execute the state change
        await sendTx("popUnicodeKey");
    }

    console.log(`\nOriginal: ${testKeys.join(", ")}`);
    console.log(`Popped:   ${poppedKeys.join(", ")}`);

    // Verify ordering matches perfectly (First In First Out)
    const matched = JSON.stringify(testKeys) === JSON.stringify(poppedKeys);
    if (matched) {
        console.log("\n>>> SUCCESS: Unicode FIFO Keyboard buffer functioning perfectly on Folklore! <<<");
    } else {
        console.error("\n>>> FAILURE: Keystroke ordering or values mismatched! <<<");
        process.exit(1);
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
