const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function bindCpuAddress(address cpu) public returns (bool)",
    "function bindMusicMaker(address musicMaker) public returns (bool)",
    "function writeReflectionCoefficients(uint256[9] coefficients, uint256 pitch, uint256 energy) public returns (bool)"
];

const musicABI = [
    "function peek(uint16 addr) external view returns (uint8)"
];

const cpuABI = [
    "function peekUser(address user, uint256 addr) public view returns (uint256)",
    "function pokeUser(address user, uint256 addr, uint256 val) public returns (uint256)"
];

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];
    const userAddr = signer.address;

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    const musicAddress = config.networks.localhost.musicMakerAddress;
    const cpuAddress = config.networks.localhost.cpu6502Address;

    const speech = new ethers.Contract(speechAddress, speechABI, signer);
    const music = new ethers.Contract(musicAddress, musicABI, signer);
    const cpu = new ethers.Contract(cpuAddress, cpuABI, signer);

    console.log("\n=== STEP 2: Binding CPU and MusicMaker ===");
    await (await speech.bindCpuAddress(cpuAddress)).wait();
    await (await speech.bindMusicMaker(musicAddress)).wait();
    console.log("Bound CPU & MusicMaker successfully.");

    // Clean up first
    await (await cpu.pokeUser(userAddr, 55043, 0)).wait();
    await (await cpu.pokeUser(userAddr, 55044, 0)).wait();

    console.log("\n=== STEP 3: Testing Physical Trauma = Exhausted (Level 1) ===");
    // Set phys_trauma = 1 (Exhausted) -> Should reduce energy to 80%
    await (await cpu.pokeUser(userAddr, 55043, 1)).wait();
    
    let coefficients = Array.from({ length: 9 }, (_, i) => i + 5);
    await (await speech.writeReflectionCoefficients(coefficients, 100, 100)).wait();
    
    let energyStored = await cpu.peekUser(userAddr, 54810);
    console.log(`Exhausted: Stored Energy is ${energyStored} (Expected: 80)`);
    if (Number(energyStored) !== 80) {
        throw new Error("FAIL: Exhausted energy suppression is incorrect.");
    }

    console.log("\n=== STEP 4: Testing Physical Trauma = Broken (Level 3) ===");
    // Set phys_trauma = 3 (Broken) -> Should reduce energy to 30%
    await (await cpu.pokeUser(userAddr, 55043, 3)).wait();
    await (await speech.writeReflectionCoefficients(coefficients, 100, 100)).wait();
    
    energyStored = await cpu.peekUser(userAddr, 54810);
    console.log(`Broken: Stored Energy is ${energyStored} (Expected: 30)`);
    if (Number(energyStored) !== 30) {
        throw new Error("FAIL: Broken energy suppression is incorrect.");
    }

    console.log("\n=== STEP 5: Testing Mental Trauma = Terrified (Level 2) ===");
    // Reset physical trauma
    await (await cpu.pokeUser(userAddr, 55043, 0)).wait();
    // Set ment_trauma = 2 (Terrified) -> Pitch should be raised (+20) and have jitter applied
    await (await cpu.pokeUser(userAddr, 55044, 2)).wait();
    
    await (await speech.writeReflectionCoefficients(coefficients, 100, 100)).wait();
    let pitchStored = await cpu.peekUser(userAddr, 54809);
    console.log(`Terrified: Stored Pitch is ${pitchStored} (Expected: ~120 with LCG jitter applied)`);
    // Check if within acceptable jitter boundary (120 - 12 to 120 + 12)
    if (Number(pitchStored) < 108 || Number(pitchStored) > 132) {
        throw new Error(`FAIL: Terrified pitch modulation of ${pitchStored} is outside bounds!`);
    }

    console.log("\n=== STEP 6: Testing Mental Trauma = Melancholic (Level 4) ===");
    // Set ment_trauma = 4 (Melancholic) -> Pitch should be lowered (-25) and energy halved (50%)
    await (await cpu.pokeUser(userAddr, 55044, 4)).wait();
    await (await speech.writeReflectionCoefficients(coefficients, 100, 100)).wait();
    
    pitchStored = await cpu.peekUser(userAddr, 54809);
    energyStored = await cpu.peekUser(userAddr, 54810);
    console.log(`Melancholic: Stored Pitch is ${pitchStored} (Expected: ~75)`);
    console.log(`Melancholic: Stored Energy is ${energyStored} (Expected: 50)`);
    if (Number(pitchStored) < 65 || Number(pitchStored) > 85) {
        throw new Error("FAIL: Melancholic pitch bounds incorrect.");
    }
    if (Number(energyStored) !== 50) {
        throw new Error("FAIL: Melancholic energy bounds incorrect.");
    }

    console.log("\n=== ALL VAESEN SPEECH HARDWARE MODULATION TESTS PASSED ===");
}

main().catch((err) => {
    console.error(err);
    process.exit(1);
});
