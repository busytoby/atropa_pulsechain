const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Delta Modulation ADC/DAC Codec (adc_tts.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/adc_tts.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/adc_tts.yul");
    const solcOutput = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const adcttsABI = [
        "function encodeDeltaMod(bytes calldata pcmData, uint8 stepSize) external view returns (bytes memory)",
        "function decodeDeltaMod(bytes calldata encodedData, uint8 stepSize, uint256 length) external view returns (bytes memory)"
    ];

    const factory = new ethers.ContractFactory(adcttsABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`ADCTTS contract deployed at: ${contractAddr}\n`);

    // Create a 16-sample ramping and falling test PCM wave
    // Midpoint is 128
    const inputWave = [
        130, 140, 150, 160, 170, 160, 150, 140,
        130, 120, 110, 100, 90,  100, 110, 120
    ];
    console.log(`Original PCM Wave (16 samples): [${inputWave.join(", ")}]`);

    // Encode
    console.log("Encoding wave with stepSize = 10...");
    const pcmBytes = Uint8Array.from(inputWave);
    const encodedHex = await contract.encodeDeltaMod(pcmBytes, 10);
    const encoded = ethers.getBytes(encodedHex);
    console.log(`Encoded packed data: [${Array.from(encoded).map(x => "0x" + x.toString(16).padStart(2, "0")).join(", ")}]`);
    console.log(`Compression ratio: 16 bytes -> ${encoded.length} bytes (8:1 compression)`);
    if (encoded.length !== 2) {
        throw new Error("Expected 2 packed bytes for 16 samples!");
    }

    // Decode
    console.log("Decoding packed data back to PCM...");
    const decodedHex = await contract.decodeDeltaMod(encodedHex, 10, 16);
    const decoded = Array.from(ethers.getBytes(decodedHex));
    console.log(`Decoded PCM Wave:              [${decoded.join(", ")}]`);

    // Basic assertion: reconstructed PCM should track the trends of the original wave.
    // Let's check that sample 4 is higher than 128, and sample 12 is lower than 128.
    console.log(`Sample 4 decoded: ${decoded[4]} (Expected: > 128)`);
    console.log(`Sample 12 decoded: ${decoded[12]} (Expected: < 128)`);

    if (decoded[4] <= 128 || decoded[12] >= 128) {
        throw new Error("Delta modulation reconstruction is incorrect!");
    }

    console.log("Passed!\n");
    console.log(">>> ALL DELTA MODULATION CODEC TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
