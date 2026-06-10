const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

async function main() {
    console.log("=== Testing SpeechSynthesis Neural/Ana Vocoder Pipeline ===");
    
    // Connect to local Anvil node
    const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
    const signer = new ethers.Wallet("0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80", provider);
    
    // Read contract addresses
    const configPath = path.join(__dirname, "../config/user_config.json");
    if (!fs.existsSync(configPath)) {
        console.error("user_config.json not found!");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(configPath, "utf8"));
    const speechAddress = config.networks.localhost.speechSynthesizerAddress;
    console.log(`SpeechSynthesis deployed address: ${speechAddress}`);
    
    // Get ABI from compiled zmachine.yul mapping
    const zmachineAbi = [
        "function setQuantizedWeights(uint256 blockIndex, uint256 packedWeights) external",
        "function registerSpeaker(bytes32 name, bytes32[8] embedding) external",
        "function predictMelQuantized(bytes32[] phonemeSequence, bytes32 speakerName) external view returns (bytes memory)",
        "function synthesizeNeuralWav(bytes melSpectrogram, uint256 upsampleFactor) external view returns (bytes memory)"
    ];
    
    const speechContract = new ethers.Contract(speechAddress, zmachineAbi, signer);
    
    // 1. Pack and upload quantized INT8 weights (32 values per block)
    console.log("\n1. Uploading quantized weights...");
    const packWeights = (weights32) => {
        let word = 0n;
        for (let i = 0; i < 32; i++) {
            let val = weights32[i] || 0;
            if (val < 0) {
                val = 256 + val;
            }
            word = word | (BigInt(val) << BigInt(i * 8));
        }
        return "0x" + word.toString(16);
    };
    
    const weightsBlock0 = [
        50, -20, 10, 0, 5, -5, 10, -10,
        -10, 60, -20, 15, 0, 10, -5, 5,
        20, -10, 70, -30, 10, 5, -10, 15,
        -5, 15, -15, 80, -20, 10, 0, 5
    ];
    const weightsBlock1 = [
        10, 0, 15, -10, 65, -15, 20, -20,
        -15, 10, 5, 10, -10, 75, -5, 10,
        5, -5, -10, 5, 15, -10, 85, -25,
        -10, 10, 20, -15, -20, 15, -15, 90
    ];
    
    let tx = await speechContract.setQuantizedWeights(0, packWeights(weightsBlock0));
    await tx.wait();
    console.log("Uploaded weights block 0");
    
    tx = await speechContract.setQuantizedWeights(1, packWeights(weightsBlock1));
    await tx.wait();
    console.log("Uploaded weights block 1");
    
    // 2. Register Speaker profile "ana"
    console.log("\n2. Registering speaker 'ana'...");
    const speakerName = ethers.encodeBytes32String("ana");
    const embedding = [
        ethers.toBeHex(12, 32),
        ethers.toBeHex(ethers.toTwos(-8, 256), 32),
        ethers.toBeHex(15, 32),
        ethers.toBeHex(0, 32),
        ethers.toBeHex(ethers.toTwos(-10, 256), 32),
        ethers.toBeHex(5, 32),
        ethers.toBeHex(ethers.toTwos(-5, 256), 32),
        ethers.toBeHex(8, 32)
    ];
    tx = await speechContract.registerSpeaker(speakerName, embedding);
    await tx.wait();
    console.log("Speaker 'ana' registered.");
    
    // 3. Formulate sequence and predict Mel-spectrogram
    console.log("\n3. Predicting Mel-spectrogram for phoneme sequence: ['ee', 'oo', 'm', 'oo', 's']...");
    const eeKey = ethers.concat([ethers.toUtf8Bytes("ee"), new Uint8Array(30)]);
    const ooKey = ethers.concat([ethers.toUtf8Bytes("oo"), new Uint8Array(30)]);
    const mKey = ethers.concat([ethers.toUtf8Bytes("m"), new Uint8Array(31)]);
    const sKey = ethers.concat([ethers.toUtf8Bytes("s"), new Uint8Array(31)]);
    const phonemeSequence = [eeKey, ooKey, mKey, ooKey, sKey];
    
    const melSpectrogramHex = await speechContract.predictMelQuantized(phonemeSequence, speakerName);
    console.log(`Mel-spectrogram predicted successfully. Length of returned bytes: ${ethers.getBytes(melSpectrogramHex).length}`);
    
    // 4. Synthesize WAV using neural vocoder mapping
    console.log("\n4. Running neural WAV synthesis...");
    const wavBytesHex = await speechContract.synthesizeNeuralWav(melSpectrogramHex, 2560, { gasLimit: 30000000 });
    const wavBytes = ethers.getBytes(wavBytesHex);
    console.log(`Neural WAV synthesis complete. WAV file size: ${wavBytes.length} bytes.`);
    
    // Verify WAV format header
    const wavHeaderString = String.fromCharCode(...wavBytes.slice(0, 4)) + 
                            " ... " + 
                            String.fromCharCode(...wavBytes.slice(8, 16));
    console.log(`WAV file header validation: ${wavHeaderString}`);
    if (String.fromCharCode(...wavBytes.slice(0, 4)) === "RIFF" && String.fromCharCode(...wavBytes.slice(8, 12)) === "WAVE") {
        console.log("SUCCESS: RIFF/WAVE header verified!");
    } else {
        console.error("FAILURE: Invalid WAV header format.");
        process.exit(1);
    }
}

main().catch(err => {
    console.error("Error during test:", err);
    process.exit(1);
});
