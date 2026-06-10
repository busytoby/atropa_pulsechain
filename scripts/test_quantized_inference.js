const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function registerSpeaker(bytes32 name, bytes32[8] memory embedding) public",
    "function setQuantizedWeights(uint256 slotOffset, uint256 packedWeights) public",
    "function predictMelSpeaker(bytes32[] memory phonemes, bytes32 name) public view returns (bytes memory)",
    "function predictMelQuantized(bytes32[] memory phonemes, bytes32 name) public view returns (bytes memory)"
];

// Helper to pack signed INT8 weights into a uint256 word
function packWeights(weights32) {
    let word = 0n;
    for (let i = 0; i < 32; i++) {
        let val = weights32[i] || 0;
        if (val < 0) {
            val = 256 + val;
        }
        word = word | (BigInt(val) << BigInt(i * 8));
    }
    return word;
}

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log("Speech Synthesizer:", speechAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);

    console.log("\n=== STEP 2: Packing and Uploading INT8 Weights ===");
    // Row 0-3 weights (32 elements)
    const weightsA = [
        // Row 0
        50, -20, 10, 0, 5, -5, 10, -10,
        // Row 1
        -10, 60, -20, 15, 0, 10, -5, 5,
        // Row 2
        20, -10, 70, -30, 10, 5, -10, 15,
        // Row 3
        -5, 15, -15, 80, -20, 10, 0, 5
    ];
    // Row 4-7 weights (32 elements)
    const weightsB = [
        // Row 4
        10, 0, 15, -10, 65, -15, 20, -20,
        // Row 5
        -15, 10, 5, 10, -10, 75, -5, 10,
        // Row 6
        5, -5, -10, 5, 15, -10, 85, -25,
        // Row 7
        -10, 10, 20, -15, -20, 15, -15, 90
    ];

    const packed0 = packWeights(weightsA);
    const packed1 = packWeights(weightsB);

    console.log("Packed Word 0:", "0x" + packed0.toString(16));
    console.log("Packed Word 1:", "0x" + packed1.toString(16));

    const tx0 = await speech.setQuantizedWeights(0, packed0);
    await tx0.wait();
    const tx1 = await speech.setQuantizedWeights(1, packed1);
    await tx1.wait();
    console.log("✓ Success: Uploaded packed INT8 weights.");

    console.log("\n=== STEP 3: Register Speaker 'ana' ===");
    const anaName = ethers.encodeBytes32String("ana");
    const anaEmbedding = [
        ethers.toBeHex(12, 32),
        ethers.toBeHex(ethers.toTwos(-8, 256), 32),
        ethers.toBeHex(15, 32),
        ethers.toBeHex(0, 32),
        ethers.toBeHex(ethers.toTwos(-10, 256), 32),
        ethers.toBeHex(5, 32),
        ethers.toBeHex(ethers.toTwos(-5, 256), 32),
        ethers.toBeHex(8, 32)
    ];
    const regTx = await speech.registerSpeaker(anaName, anaEmbedding);
    await regTx.wait();

    console.log("\n=== STEP 4: Compare Hardcoded Speaker Mel vs Quantized Mel ===");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const phonemes = [eeKey, ooKey];

    const speakerHex = await speech.predictMelSpeaker(phonemes, anaName);
    const quantizedHex = await speech.predictMelQuantized(phonemes, anaName);

    console.log("Speaker baseline prediction:", speakerHex);
    console.log("Quantized dynamic prediction:", quantizedHex);

    if (speakerHex !== quantizedHex) {
        throw new Error("FAIL: Quantized prediction output does not match the reference speaker model output!");
    }
    console.log("✓ Success: Quantized inference engine outputs match reference output exactly!");
    console.log("\n=== QUANTIZED INFERENCE SYSTEM VERIFIED SUCCESS ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
