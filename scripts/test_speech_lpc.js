const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function bindCpuAddress(address cpu) public returns (bool)",
    "function getSpeechState() public view returns (uint256, uint256, uint256, uint256)",
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

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    const musicAddress = config.networks.localhost.musicMakerAddress;
    const cpuAddress = config.networks.localhost.cpu6502Address;
    console.log("Speech Synthesizer:", speechAddress);
    console.log("Music Maker:", musicAddress);
    console.log("CPU 6502 Address:", cpuAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);
    const music = new ethers.Contract(musicAddress, musicABI, signer);
    const cpu = new ethers.Contract(cpuAddress, cpuABI, signer);

    console.log("\n=== STEP 2: Binding CPU Address ===");
    await (await speech.bindCpuAddress(cpuAddress)).wait();
    console.log("CPU bound successfully.");

    console.log("\n=== STEP 3: Binding MusicMaker ===");
    const bindTx = await speech.bindMusicMaker(musicAddress);
    await bindTx.wait();
    console.log("MusicMaker bound successfully.");

    // Reset any residual trauma
    const userAddr = signer.address;
    await (await cpu.pokeUser(userAddr, 55043, 0)).wait();
    await (await cpu.pokeUser(userAddr, 55044, 0)).wait();

    console.log("\n=== STEP 4: Writing Neural LPC Reflection Coefficients ===");
    // Test values: Reflection coefficients K1..K9, pitch = 120, energy = 40
    const mockCoefficients = Array.from({ length: 9 }, (_, i) => i + 5);
    const testPitch = 120;
    const testEnergy = 40;

    console.log("Coefficients:", mockCoefficients);
    console.log("Pitch:", testPitch, "Energy:", testEnergy);

    const writeTx1 = await speech.writeReflectionCoefficients(mockCoefficients, testPitch, testEnergy);
    await writeTx1.wait();
    const writeTx2 = await speech.writeReflectionCoefficients(mockCoefficients, testPitch, testEnergy);
    await writeTx2.wait();
    console.log("Reflection coefficients written (double-called for pitch smoothing convergence).");

    console.log("\n=== STEP 5: Verifying CPU Memory Pokes ===");
    // Reflection coefficients K1..K9 are written to 54800-54808 in caller's memory
    console.log(`Checking memory for user: ${userAddr}`);
    
    for (let i = 0; i < 9; i++) {
        const val = await cpu.peekUser(userAddr, 54800 + i);
        console.log(`K${i+1} at ${54800 + i}: ${val} (Expected: ${mockCoefficients[i]})`);
        if (Number(val) !== mockCoefficients[i]) {
            throw new Error(`FAIL: K${i+1} mismatch!`);
        }
    }

    const pitchVal = await cpu.peekUser(userAddr, 54809);
    const energyVal = await cpu.peekUser(userAddr, 54810);
    console.log(`Pitch at 54810: ${pitchVal} (Expected: ${testPitch})`);
    console.log(`Energy at 54811: ${energyVal} (Expected: ${testEnergy})`);

    if (Number(pitchVal) !== testPitch || Number(energyVal) !== testEnergy) {
        throw new Error("FAIL: Pitch or Energy mismatch in CPU user memory!");
    }
    console.log("SUCCESS: CPU memory pokes verified.");

    console.log("\n=== STEP 6: Verifying MusicMaker (SID Vocoder) modulation ===");
    // Pitch = 120 -> freq := 120 * 10 = 1200
    // lo := 1200 & 0xff = 176
    // hi := (1200 >> 8) & 0xff = 4
    const sidFreqLo = await music.peek(54272);
    const sidFreqHi = await music.peek(54273);
    const sidCtrl = await music.peek(54276);

    console.log(`SID Freq Low: ${sidFreqLo} (Expected: 176)`);
    console.log(`SID Freq High: ${sidFreqHi} (Expected: 4)`);
    console.log(`SID Voice 1 Ctrl: ${sidCtrl} (Expected: 17)`);

    if (Number(sidFreqLo) !== 176 || Number(sidFreqHi) !== 4 || Number(sidCtrl) !== 17) {
        throw new Error("FAIL: SID frequency modulation registers mismatch!");
    }
    console.log("SUCCESS: SID Vocoder modulation verified successfully!");

    console.log("\n=== ALL NEURAL LPC AND SID VOCODER UPGRADE TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
