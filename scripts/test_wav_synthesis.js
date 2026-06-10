const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function selectVoice(uint256 voiceId) public returns (bool)",
    "function synthesizeWav(bytes32 phonemeKey, uint256 numSamples) public view returns (bytes)"
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

    console.log("\n=== STEP 2: Generating 'aa' WAV file ===");
    const keyAa = ethers.concat([ethers.toUtf8Bytes("aa"), new Uint8Array(30)]);
    const wavBytesHex = await speech.synthesizeWav(keyAa, 100);
    
    const wavBuffer = Buffer.from(wavBytesHex.substring(2), "hex");
    console.log("WAV output total bytes:", wavBuffer.length);

    console.log("\n=== STEP 3: Decoding WAV Header (first 44 bytes) ===");
    const riff = wavBuffer.subarray(0, 4).toString("utf8");
    const chunkSize = wavBuffer.readUInt32LE(4);
    const wave = wavBuffer.subarray(8, 12).toString("utf8");
    const fmt = wavBuffer.subarray(12, 16).toString("utf8");
    const subchunk1Size = wavBuffer.readUInt32LE(16);
    const audioFormat = wavBuffer.readUInt16LE(20);
    const numChannels = wavBuffer.readUInt16LE(22);
    const sampleRate = wavBuffer.readUInt32LE(24);
    const byteRate = wavBuffer.readUInt32LE(28);
    const blockAlign = wavBuffer.readUInt16LE(32);
    const bitsPerSample = wavBuffer.readUInt16LE(34);
    const dataMarker = wavBuffer.subarray(36, 40).toString("utf8");
    const subchunk2Size = wavBuffer.readUInt32LE(40);

    console.log(`RIFF Marker:       ${riff} (Expected: RIFF)`);
    console.log(`Chunk Size:        ${chunkSize} (Expected: 236)`);
    console.log(`WAVE Marker:       ${wave} (Expected: WAVE)`);
    console.log(`fmt Marker:        ${fmt} (Expected: fmt )`);
    console.log(`Subchunk1 Size:    ${subchunk1Size} (Expected: 16)`);
    console.log(`Audio Format:      ${audioFormat} (Expected: 1)`);
    console.log(`Channels:          ${numChannels} (Expected: 1)`);
    console.log(`Sample Rate:       ${sampleRate} (Expected: 16000)`);
    console.log(`Byte Rate:         ${byteRate} (Expected: 32000)`);
    console.log(`Block Align:       ${blockAlign} (Expected: 2)`);
    console.log(`Bits Per Sample:   ${bitsPerSample} (Expected: 16)`);
    console.log(`data Marker:       ${dataMarker} (Expected: data)`);
    console.log(`Subchunk2 Size:    ${subchunk2Size} (Expected: 200)`);

    // Verify WAV compliance
    if (riff !== "RIFF" || wave !== "WAVE" || fmt !== "fmt " || dataMarker !== "data") {
        throw new Error("FAIL: Invalid WAV format markers!");
    }
    if (chunkSize !== 236 || subchunk2Size !== 200) {
        throw new Error(`FAIL: Invalid chunk sizes! (Got chunkSize: ${chunkSize}, subchunk2Size: ${subchunk2Size})`);
    }
    if (sampleRate !== 16000 || byteRate !== 32000) {
        throw new Error("FAIL: Invalid sample rate or byte rate!");
    }

    console.log("✓ Success: WAV headers fully validated and compliant with standard RIFF/Wave format.");

    console.log("\n=== STEP 4: Checking PCM payload samples (after byte 44) ===");
    const pcmSamples = new Int16Array(wavBuffer.subarray(44).buffer, wavBuffer.subarray(44).byteOffset, 100);
    console.log("First 15 decoded PCM samples (little-endian decoded):");
    console.log(Array.from(pcmSamples.slice(0, 15)));

    let nonZero = false;
    for (let s of pcmSamples) {
        if (Math.abs(s) > 1) {
            nonZero = true;
            break;
        }
    }
    if (!nonZero) {
        throw new Error("FAIL: WAV payload is entirely silent!");
    }
    console.log("✓ Success: Non-zero audio samples found in WAV payload.");

    console.log("\n=== ALL WAV SYNTHESIS PACKAGING ENCODER TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
