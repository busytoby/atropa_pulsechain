const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function predictMel(bytes32[] memory phonemes) public view returns (bytes memory)"
];

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log("Speech Synthesizer:", speechAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);

    console.log("\n=== STEP 2: Testing predictMel with ['ee', 'oo', 's'] ===");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const sKey = ethers.concat([ethers.toUtf8Bytes("s"), new Uint8Array(31)]);

    const melBytesHex = await speech.predictMel([eeKey, ooKey, sKey]);
    
    const melBuffer = Buffer.from(melBytesHex.substring(2), "hex");
    console.log("Mel-Spectrogram output buffer length (bytes):", melBuffer.length);

    if (melBuffer.length !== 24) {
        throw new Error(`FAIL: Expected 24 bytes (3 frames * 8 bands), but got ${melBuffer.length}!`);
    }
    console.log("✓ Success: Correct output byte size.");

    console.log("\n=== STEP 3: Decoding Mel-Spectrogram Frames ===");
    for (let frameIdx = 0; frameIdx < 3; frameIdx++) {
        const frameBytes = melBuffer.subarray(frameIdx * 8, frameIdx * 8 + 8);
        const frameVals = Array.from(frameBytes);
        const name = ["ee", "oo", "s"][frameIdx];
        console.log(`Frame ${frameIdx + 1} ("${name}") Mel Bands (0-7):`, frameVals);
        
        let sum = frameVals.reduce((a, b) => a + b, 0);
        if (sum === 0) {
            throw new Error(`FAIL: Frame ${frameIdx + 1} is completely empty!`);
        }
    }

    console.log("\n=== ALL ON-CHAIN NEURAL ACOUSTIC MODEL PREDICTOR TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
