const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function bindCpuAddress(address cpu) public returns (bool)",
    "function bindMusicMaker(address musicMaker) public returns (bool)",
    "function writeFormants(uint256 f1, uint256 f2, uint256 f3) public returns (bool)"
];

const musicABI = [
    "function peek(uint16 addr) public view returns (uint256)"
];

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    const musicAddress = config.networks.localhost.musicMakerAddress;
    const cpuAddress = config.networks.localhost.cpu6502Address;

    const speech = new ethers.Contract(speechAddress, speechABI, signer);
    const music = new ethers.Contract(musicAddress, musicABI, signer);

    console.log("\n=== STEP 2: Binding MusicMaker ===");
    await (await speech.bindMusicMaker(musicAddress)).wait();
    console.log("Bound MusicMaker successfully.");

    console.log("\n=== STEP 3: Writing Formant Frequencies ===");
    // Let's use test frequencies:
    // f1 = 500 Hz -> freq1 = 5000 -> lo1 = 5000 % 256 = 136, hi1 = floor(5000 / 256) = 19
    // f2 = 1500 Hz -> freq2 = 15000 -> lo2 = 15000 % 256 = 152, hi2 = floor(15000 / 256) = 58
    // f3 = 2500 Hz -> freq3 = 25000 -> lo3 = 25000 % 256 = 168, hi3 = floor(25000 / 256) = 97
    const f1 = 500;
    const f2 = 1500;
    const f3 = 2500;

    const tx = await speech.writeFormants(f1, f2, f3);
    await tx.wait();
    console.log(`Wrote formants: f1=${f1}, f2=${f2}, f3=${f3}`);

    console.log("\n=== STEP 4: Verifying Voice Registers on MusicMaker ===");
    
    // Voice 1
    const v1_lo = await music.peek(54272);
    const v1_hi = await music.peek(54273);
    const v1_ctrl = await music.peek(54276);
    console.log(`Voice 1: Freq Low = ${v1_lo} (Expected: 136), Freq High = ${v1_hi} (Expected: 19), Control = ${v1_ctrl} (Expected: 17)`);

    // Voice 2
    const v2_lo = await music.peek(54279);
    const v2_hi = await music.peek(54280);
    const v2_ctrl = await music.peek(54283);
    console.log(`Voice 2: Freq Low = ${v2_lo} (Expected: 152), Freq High = ${v2_hi} (Expected: 58), Control = ${v2_ctrl} (Expected: 17)`);

    // Voice 3
    const v3_lo = await music.peek(54286);
    const v3_hi = await music.peek(54287);
    const v3_ctrl = await music.peek(54290);
    console.log(`Voice 3: Freq Low = ${v3_lo} (Expected: 168), Freq High = ${v3_hi} (Expected: 97), Control = ${v3_ctrl} (Expected: 17)`);

    // Assertions
    if (Number(v1_lo) !== 136 || Number(v1_hi) !== 19 || Number(v1_ctrl) !== 17) {
        throw new Error("Voice 1 register check failed.");
    }
    if (Number(v2_lo) !== 152 || Number(v2_hi) !== 58 || Number(v2_ctrl) !== 17) {
        throw new Error("Voice 2 register check failed.");
    }
    if (Number(v3_lo) !== 168 || Number(v3_hi) !== 97 || Number(v3_ctrl) !== 17) {
        throw new Error("Voice 3 register check failed.");
    }

    console.log("\n=== ALL FORMANT SPEECH SYNTHESIS TESTS PASSED ===");
}

main().catch((err) => {
    console.error(err);
    process.exit(1);
});
