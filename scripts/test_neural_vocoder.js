const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function predictMel(bytes32[] memory phonemes) public view returns (bytes memory)",
    "function synthesizeNeuralWav(bytes memory melFrames, uint256 upsampleFactor) public view returns (bytes memory)"
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

    console.log("\n=== STEP 2: Predict Mel-Spectrogram frames for 'ee' + 'oo' + 'm' + 'oo' + 's' ===");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const mKey = ethers.concat([ethers.toUtf8Bytes("m"), new Uint8Array(31)]);
    const sKey = ethers.concat([ethers.toUtf8Bytes("s"), new Uint8Array(31)]);

    const phonemeSeq = [eeKey, ooKey, mKey, ooKey, sKey];
    const melBytesHex = await speech.predictMel(phonemeSeq);
    console.log("Generated Mel-Spectrogram Bytes length:", (melBytesHex.length - 2) / 2);

    console.log("\n=== STEP 3: Feeding Mel-Spectrogram into Neural Vocoder ===");
    const upsampleFactor = 80; // 80 audio samples per frame
    const wavBytesHex = await speech.synthesizeNeuralWav(melBytesHex, upsampleFactor);
    
    const wavBuffer = Buffer.from(wavBytesHex.substring(2), "hex");
    console.log("Synthesized WAV file size:", wavBuffer.length, "bytes");

    const destPath = path.join(__dirname, "../neural_synthesis.wav");
    fs.writeFileSync(destPath, wavBuffer);
    console.log("Neural synthesis output saved to:", destPath);

    console.log("\n=== NEURAL VOCODER PIPELINE VERIFICATION SUCCESS ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
