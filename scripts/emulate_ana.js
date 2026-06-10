const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function registerSpeaker(bytes32 name, bytes32[8] memory embedding) public",
    "function setQuantizedWeights(uint256 slotOffset, uint256 packedWeights) public",
    "function predictMelQuantized(bytes32[] memory phonemes, bytes32 name) public view returns (bytes memory)",
    "function synthesizeNeuralWav(bytes memory melFrames, uint256 upsampleFactor) public view returns (bytes memory)"
];

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

    console.log("\n=== STEP 2: Loading & Uploading Model Weights ===");
    const weightsA = [
        50, -20, 10, 0, 5, -5, 10, -10,
        -10, 60, -20, 15, 0, 10, -5, 5,
        20, -10, 70, -30, 10, 5, -10, 15,
        -5, 15, -15, 80, -20, 10, 0, 5
    ];
    const weightsB = [
        10, 0, 15, -10, 65, -15, 20, -20,
        -15, 10, 5, 10, -10, 75, -5, 10,
        5, -5, -10, 5, 15, -10, 85, -25,
        -10, 10, 20, -15, -20, 15, -15, 90
    ];

    const packed0 = packWeights(weightsA);
    const packed1 = packWeights(weightsB);

    const tx0 = await speech.setQuantizedWeights(0, packed0);
    await tx0.wait();
    const tx1 = await speech.setQuantizedWeights(1, packed1);
    await tx1.wait();
    console.log("✓ Packed INT8 Model Weights successfully written to Storage.");

    console.log("\n=== STEP 3: Registering Latent Speaker Blueprint for 'ana' ===");
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
    console.log("✓ Latent Speaker Embedding successfully registered for 'ana'.");

    console.log("\n=== STEP 4: Generating Ana-Conditioned Mel-Spectrogram ===");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const mKey = ethers.concat([ethers.toUtf8Bytes("m"), new Uint8Array(31)]);
    const sKey = ethers.concat([ethers.toUtf8Bytes("s"), new Uint8Array(31)]);
    const phonemeSeq = [eeKey, ooKey, mKey, ooKey, sKey];

    const melBytesHex = await speech.predictMelQuantized(phonemeSeq, anaName);
    console.log("✓ Generated Ana-conditioned Mel-spectrogram bytes (length):", (melBytesHex.length - 2) / 2);

    console.log("\n=== STEP 5: Feeding Spectrogram to Neural Vocoder ===");
    const upsampleFactor = 80;
    const wavBytesHex = await speech.synthesizeNeuralWav(melBytesHex, upsampleFactor);
    const wavBuffer = Buffer.from(wavBytesHex.substring(2), "hex");
    console.log("✓ Wave synthesis complete. WAV file size:", wavBuffer.length, "bytes");

    const destPath = path.join(__dirname, "../ana_emulated.wav");
    fs.writeFileSync(destPath, wavBuffer);
    console.log("✓ Output file saved to:", destPath);

    console.log("\n=== ELEVENLABS ANA VOICE EMULATION SUCCESSFUL ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
