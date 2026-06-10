const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function selectVoice(uint256 voiceId) public returns (bool)",
    "function synthesizeAudio(bytes32 phonemeKey, uint256 numSamples) public view returns (bytes)"
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

    console.log("\n=== STEP 2: Selecting Ana Voice (ID 0) ===");
    await (await speech.selectVoice(0)).wait();
    console.log("Ana voice selected.");

    console.log("\n=== STEP 3: Generating Voiced Vowel 'aa' (100 samples) ===");
    const keyAa = ethers.concat([ethers.toUtf8Bytes("aa"), new Uint8Array(30)]);
    
    // Call synthesizeAudio (read-only view)
    const resultBytes = await speech.synthesizeAudio(keyAa, 100);
    console.log("Returned bytes length:", resultBytes.length, "chars");

    // Convert hex string to Uint8Array and parse as 16-bit signed integers
    const dataHex = resultBytes.substring(2); // strip "0x"
    const pcmData = new Int16Array(dataHex.length / 4);
    
    for (let i = 0; i < pcmData.length; i++) {
        // Read 16-bit big-endian signed integer
        const sampleHex = dataHex.substring(i * 4, i * 4 + 4);
        let val = parseInt(sampleHex, 16);
        if (val >= 0x8000) val -= 0x10000;
        pcmData[i] = val;
    }

    console.log("First 15 synthesized 16-bit PCM samples:");
    console.log(Array.from(pcmData.slice(0, 15)));

    // Verify filter activity (non-zero output)
    let nonZero = false;
    for (let s of pcmData) {
        if (Math.abs(s) > 1) {
            nonZero = true;
            break;
        }
    }

    if (!nonZero) {
        throw new Error("FAIL: Synthesizer returned completely silent/zero samples!");
    }
    console.log("✓ Success: Non-zero audio samples generated on-chain.");

    console.log("\n=== STEP 4: Selecting Moloch Voice (ID 1) & Synthesizing 'sh' (80 samples) ===");
    await (await speech.selectVoice(1)).wait();
    const keySh = ethers.concat([ethers.toUtf8Bytes("sh"), new Uint8Array(30)]);
    const shBytes = await speech.synthesizeAudio(keySh, 80);
    
    const shHex = shBytes.substring(2);
    const shPcm = new Int16Array(shHex.length / 4);
    for (let i = 0; i < shPcm.length; i++) {
        let val = parseInt(shHex.substring(i * 4, i * 4 + 4), 16);
        if (val >= 0x8000) val -= 0x10000;
        shPcm[i] = val;
    }

    console.log("First 15 sibilant PCM samples:");
    console.log(Array.from(shPcm.slice(0, 15)));

    let shNonZero = false;
    for (let s of shPcm) {
        if (Math.abs(s) > 1) {
            shNonZero = true;
            break;
        }
    }
    if (!shNonZero) {
        throw new Error("FAIL: Moloch 'sh' synthesis returned silent samples!");
    }
    console.log("✓ Success: Moloch unvoiced sibilant samples generated on-chain.");

    console.log("\n=== ALL ON-CHAIN AUDIO SYNTHESIS EMULATION TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
