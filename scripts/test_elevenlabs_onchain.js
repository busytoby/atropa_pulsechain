const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

const speechABI = [
    "function bindCpuAddress(address cpu) public returns (bool)",
    "function bindMusicMaker(address musicMaker) public returns (bool)",
    "function selectVoice(uint256 voiceId) public returns (bool)",
    "function generateNeuralSpeech(bytes32 phonemeKey) public returns (bool)"
];

const cpuABI = [
    "function peekUser(address user, uint256 addr) public view returns (uint256)"
];

async function main() {
    console.log("=== STEP 1: Connecting to Local Provider ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];
    const userAddr = signer.address;

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    const cpuAddress = config.networks.localhost.cpu6502Address;
    const musicAddress = config.networks.localhost.musicMakerAddress;

    console.log("Speech Synthesizer:", speechAddress);
    console.log("CPU 6502 Address:", cpuAddress);

    const speech = new ethers.Contract(speechAddress, speechABI, signer);
    const cpu = new ethers.Contract(cpuAddress, cpuABI, signer);

    console.log("\n=== STEP 2: Binding CPU and MusicMaker ===");
    await (await speech.bindCpuAddress(cpuAddress)).wait();
    await (await speech.bindMusicMaker(musicAddress)).wait();
    console.log("Bound CPU and MusicMaker successfully.");

    console.log("\n=== STEP 3: Selecting Moloch Voice & Generating Vowel 'ee' ===");
    // selectVoice(1) -> Moloch
    await (await speech.selectVoice(1)).wait();
    
    // generateNeuralSpeech("ee")
    const keyEe = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    await (await speech.generateNeuralSpeech(keyEe)).wait();
    console.log("Neural speech generated for 'ee' (Moloch voice).");

    console.log("\n=== STEP 4: Verifying CPU Memory Pokes for Moloch + 'ee' ===");
    // Moloch base K coefficients adjusted for voiced vowel (formant mod +15 to first 3 coeffs)
    // Moloch base: K1=85, K2=-65 (2s complement), K3=70
    // Voiced ee: adds +15 to first 3 coefficients.
    // Result: K1 = 85 + 15 = 100 -> clamped to 99.
    //         K2 = -65 + 15 = -50 -> stored as 2s complement or negative representation.
    //         K3 = 70 + 15 = 85.
    const expectedMolochEe = [99, -50, 85, -45, 35, -22, 18, -10, 5];
    
    for (let i = 0; i < 9; i++) {
        const val = await cpu.peekUser(userAddr, 54800 + i);
        let signedBig = BigInt(val);
        if (signedBig > (1n << 255n)) {
            signedBig = signedBig - (1n << 260n) === 0n ? signedBig : signedBig - (1n << 256n);
        }
        let signedVal = Number(signedBig);
        console.log(`K${i+1}: ${signedVal} (Expected: ${expectedMolochEe[i]})`);
        
        if (signedVal !== expectedMolochEe[i]) {
            throw new Error(`FAIL: K${i+1} mismatch! Expected ${expectedMolochEe[i]}, got ${signedVal}`);
        }
    }

    const pitchEe = await cpu.peekUser(userAddr, 54809);
    const energyEe = await cpu.peekUser(userAddr, 54810);
    console.log(`Pitch: ${pitchEe} (Expected: 85)`);
    console.log(`Energy: ${energyEe} (Expected: 95)`);
    
    if (Number(pitchEe) !== 85 || Number(energyEe) !== 95) {
        throw new Error("FAIL: Pitch or Energy mismatch for Moloch + 'ee'!");
    }

    console.log("\n=== STEP 5: Selecting Ana Voice & Generating Sibilant 'sh' ===");
    // selectVoice(0) -> Ana
    await (await speech.selectVoice(0)).wait();
    
    // generateNeuralSpeech("sh")
    const keySh = ethers.concat([ethers.toUtf8Bytes("sh"), new Uint8Array(30)]);
    await (await speech.generateNeuralSpeech(keySh)).wait();
    console.log("Neural speech generated for 'sh' (Ana voice).");

    console.log("\n=== STEP 6: Verifying CPU Memory Pokes for Ana + 'sh' ===");
    // Ana base K coefficients: K1=15, K2=-25, K3=40, K4=-10
    // Unvoiced sh: subtracts -40 from first 4 coefficients
    // Result: K1 = 15 - 40 = -25.
    //         K2 = -25 - 40 = -65.
    //         K3 = 40 - 40 = 0.
    //         K4 = -10 - 40 = -50.
    const expectedAnaSh = [-25, -65, 0, -50, 5, -8, 12, -5, 2];

    for (let i = 0; i < 9; i++) {
        const val = await cpu.peekUser(userAddr, 54800 + i);
        let signedBig = BigInt(val);
        if (signedBig > (1n << 255n)) {
            signedBig = signedBig - (1n << 260n) === 0n ? signedBig : signedBig - (1n << 256n);
        }
        let signedVal = Number(signedBig);
        console.log(`K${i+1}: ${signedVal} (Expected: ${expectedAnaSh[i]})`);
        
        if (signedVal !== expectedAnaSh[i]) {
            throw new Error(`FAIL: K${i+1} mismatch! Expected ${expectedAnaSh[i]}, got ${signedVal}`);
        }
    }

    const pitchSh = await cpu.peekUser(userAddr, 54809);
    const energySh = await cpu.peekUser(userAddr, 54810);
    console.log(`Pitch: ${pitchSh} (Expected: 220)`);
    console.log(`Energy: ${energySh} (Expected: 80)`);

    if (Number(pitchSh) !== 220 || Number(energySh) !== 80) {
        throw new Error("FAIL: Pitch or Energy mismatch for Ana + 'sh'!");
    }

    console.log("\n=== ALL ON-CHAIN ELEVENLABS SPEECH TESTS PASSED ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
