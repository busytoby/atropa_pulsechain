const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function predictMel(bytes32[] memory phonemes) public view returns (bytes memory)",
    "function registerSpeaker(bytes32 name, bytes32[8] memory embedding) public",
    "function predictMelSpeaker(bytes32[] memory phonemes, bytes32 name) public view returns (bytes memory)"
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

    console.log("\n=== STEP 2: Register Speaker 'ana' ===");
    const anaName = ethers.encodeBytes32String("ana");
    // Define an 8-dimensional speaker embedding vector
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
    console.log("✓ Success: Registered speaker 'ana' with embedding.");

    console.log("\n=== STEP 3: Comparing baseline vs speaker-conditioned Mel-Spectrograms ===");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const sKey = ethers.concat([ethers.toUtf8Bytes("s"), new Uint8Array(31)]);
    const phonemes = [eeKey, ooKey, sKey];

    const baselineHex = await speech.predictMel(phonemes);
    const baselineBuffer = Buffer.from(baselineHex.substring(2), "hex");

    const anaHex = await speech.predictMelSpeaker(phonemes, anaName);
    const anaBuffer = Buffer.from(anaHex.substring(2), "hex");

    console.log("Baseline frame 1 (ee) Mel bands:", Array.from(baselineBuffer.subarray(0, 8)));
    console.log("Ana-shifted frame 1 (ee) Mel bands:", Array.from(anaBuffer.subarray(0, 8)));

    let hasDifference = false;
    for (let i = 0; i < 24; i++) {
        if (baselineBuffer[i] !== anaBuffer[i]) {
            hasDifference = true;
            break;
        }
    }

    if (!hasDifference) {
        throw new Error("FAIL: Speaker embedding had no effect on the predicted Mel spectrogram!");
    }
    console.log("✓ Success: Speaker embedding correctly shifted the acoustic latent projection space.");
    console.log("\n=== SPEAKER EMBEDDING LATENT BLUEPRINT TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
