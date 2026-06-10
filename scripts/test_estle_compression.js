const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing EstleCompressor (BYTE Production RLE Graphics) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/estle.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/estle.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const estleABI = [
        "function compress(bytes rawData) external view returns (bytes rleData)",
        "function decompress(bytes rleData) external view returns (bytes rawData)"
    ];

    const factory = new ethers.ContractFactory(estleABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`EstleCompressor deployed at: ${contractAddr}\n`);

    // Test Case 1: Compression
    // "AAAAABBBCC" -> 5 'A' (0x41), 3 'B' (0x42), 2 'C' (0x43)
    // Expect: 0x054103420243
    const inputStr = "AAAAABBBCC";
    console.log(`Compressing: "${inputStr}"`);
    const inputBytes = ethers.hexlify(ethers.toUtf8Bytes(inputStr));
    const compBytes = await contract.compress(inputBytes);
    console.log(`Compressed Bytes: ${compBytes}`);
    if (compBytes !== "0x054103420243") {
        console.error("FAIL: Expected 0x054103420243");
        process.exit(1);
    }
    console.log("Passed!\n");

    // Test Case 2: Decompression
    // 0x054103420243 -> "AAAAABBBCC"
    console.log(`Decompressing: 0x054103420243`);
    const decompBytes = await contract.decompress("0x054103420243");
    const decompStr = ethers.toUtf8String(decompBytes);
    console.log(`Decompressed String: "${decompStr}"`);
    if (decompStr !== inputStr) {
        console.error(`FAIL: Expected "${inputStr}", got "${decompStr}"`);
        process.exit(1);
    }
    console.log("Passed!\n");

    // Test Case 3: Long runs (> 255)
    // Let's test a run of 260 'X' characters (0x58).
    // The RLE output should be: [255, 'X'] followed by [5, 'X'] -> total 4 bytes: 0xff580558
    const longInput = "X".repeat(260);
    console.log(`Compressing 260 'X's...`);
    const longInputBytes = ethers.hexlify(ethers.toUtf8Bytes(longInput));
    const longCompBytes = await contract.compress(longInputBytes);
    console.log(`Compressed Bytes: ${longCompBytes}`);
    if (longCompBytes !== "0xff580558") {
        console.error("FAIL: Expected 0xff580558");
        process.exit(1);
    }

    console.log("Decompressing back 0xff580558...");
    const longDecompBytes = await contract.decompress(longCompBytes);
    const longDecompStr = ethers.toUtf8String(longDecompBytes);
    if (longDecompStr !== longInput) {
        console.error("FAIL: Decompressed string did not match!");
        process.exit(1);
    }
    console.log("Passed!\n");

    console.log(">>> ALL ESTLE COMPRESSOR TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
