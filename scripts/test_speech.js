const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function getSpeechState() public view returns (uint256, uint256, uint256, uint256)",
    "function writePhoneme(uint256 phoneme, uint256 inflection) public returns (uint256)",
    "function clearBusy() public returns (uint256)"
];

async function main() {
    console.log("Connecting to provider...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log("Speech Synthesizer Address:", speechAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);

    console.log("\n--- Initial State ---");
    let state = await speech.getSpeechState();
    console.log(`Phoneme: ${state[0]}, Inflection: ${state[1]}, Busy: ${state[2]}, Counter: ${state[3]}`);

    console.log("\n--- Writing Phoneme (0x02 - EH1, Inflection: 2) ---");
    const tx = await speech.writePhoneme(2, 2);
    await tx.wait();
    console.log("Phoneme written successfully.");

    state = await speech.getSpeechState();
    console.log(`Phoneme: ${state[0]}, Inflection: ${state[1]}, Busy: ${state[2]}, Counter: ${state[3]}`);
    if (Number(state[0]) === 2 && Number(state[1]) === 2 && Number(state[2]) === 1) {
        console.log("SUCCESS: State updated correctly.");
    } else {
        throw new Error("FAIL: State was not updated correctly!");
    }

    console.log("\n--- Clearing Busy ---");
    const clearTx = await speech.clearBusy();
    await clearTx.wait();
    console.log("Busy cleared.");

    state = await speech.getSpeechState();
    console.log(`Phoneme: ${state[0]}, Inflection: ${state[1]}, Busy: ${state[2]}, Counter: ${state[3]}`);
    if (Number(state[2]) === 0) {
        console.log("SUCCESS: Busy flag cleared successfully!");
    } else {
        throw new Error("FAIL: Busy flag was not cleared!");
    }
    
    console.log("\n=== ALL SPEECH SYNTHESIS TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
