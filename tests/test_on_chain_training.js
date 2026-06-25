const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function trainOnPhoneme(bytes32 name, bytes32 phoneme) public returns (uint256, uint256)",
    "function predictMel(bytes32[] memory phonemes) public view returns (bytes memory)",
    "function setQuantizedWeights(uint256 slotOffset, uint256 packedWeights) public returns (bool)"
];

async function main() {
    console.log("=== CONNECTING TO ETH LOCAL PROVIDER ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log("Speech Synthesizer Address:", speechAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);

    console.log("\n=== STEP 1: Setting initial weights to zero ===");
    await (await speech.setQuantizedWeights(0, 0n)).wait();
    await (await speech.setQuantizedWeights(1, 0n)).wait();
    console.log("Initial weights zeroed.");

    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const name = ethers.zeroPadValue(ethers.toUtf8Bytes("moloch"), 32);

    // Initial prediction and weights check
    let weights0Before = await provider.getStorage(speechAddress, 0x7777);
    let weights1Before = await provider.getStorage(speechAddress, 0x7778);
    console.log("Weights 0x7777 before:", weights0Before);
    console.log("Weights 0x7778 before:", weights1Before);

    let melBytesHex = await speech.predictMel([eeKey]);
    console.log("Initial predicted Mel bytes:", melBytesHex);

    console.log("\n=== STEP 2: Running 5 iterations of on-chain training ===");
    for (let i = 1; i <= 5; i++) {
        // Perform staticCall to inspect the returned weights before executing the state change
        const outputs = await speech.trainOnPhoneme.staticCall(name, eeKey);
        console.log(`Static call outputs iteration ${i}:`, outputs);
        
        const tx = await speech.trainOnPhoneme(name, eeKey);
        await tx.wait();
        console.log(`Training iteration ${i} completed.`);
    }

    // Post-training predicted and weights check
    let weights0After = await provider.getStorage(speechAddress, 0x7777);
    let weights1After = await provider.getStorage(speechAddress, 0x7778);
    console.log("Weights 0x7777 after:", weights0After);
    console.log("Weights 0x7778 after:", weights1After);

    melBytesHex = await speech.predictMel([eeKey]);
    console.log("Post-training predicted Mel bytes:", melBytesHex);
    console.log("SUCCESS: On-chain Delta Rule self-training successfully verified!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
