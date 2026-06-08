const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PARSER_YUL_PATH = path.join(__dirname, "../solidity/bin/musicNotationParser.yul");
const PROVIDER_URL = "http://127.0.0.1:8545";

// Helper to compile Yul
function compileYul(yulPath) {
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("=== Starting MusicNotationParser Unit Tests & Benchmarks ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error connecting to local EVM node. Make sure Anvil/Hardhat is running!");
        process.exit(1);
    }
    const deployer = signers[0];
    console.log("Deployer Address:", deployer.address);

    // 1. Compile and deploy
    console.log("\n[DEPLOY] Compiling and deploying musicNotationParser.yul...");
    const bytecode = compileYul(PARSER_YUL_PATH);
    const tx = await deployer.sendTransaction({ data: bytecode, gasLimit: 4000000 });
    const receipt = await tx.wait();
    const contractAddress = receipt.contractAddress;
    console.log("Parser contract deployed at:", contractAddress);

    const abi = [
        "function parseNote(string noteStr) public returns (uint256 frequency, uint256 waveform)"
    ];
    const contract = new ethers.Contract(contractAddress, abi, deployer);

    // 2. Run test matrix
    const testCases = [
        { note: "C4", expectedFreq: 4392, expectedWave: 0x11 },
        { note: "C4T", expectedFreq: 4392, expectedWave: 0x11 },
        { note: "C#4S", expectedFreq: 4653, expectedWave: 0x21 },
        { note: "D4P", expectedFreq: 4930, expectedWave: 0x41 },
        { note: "E4N", expectedFreq: 5534, expectedWave: 0x81 },
        { note: "A4", expectedFreq: 7386, expectedWave: 0x11 },
        { note: "A5T", expectedFreq: 7386 * 2, expectedWave: 0x11 },
        { note: "A3T", expectedFreq: Math.floor(7386 / 2), expectedWave: 0x11 },
        { note: "Bb3S", expectedFreq: 3912, expectedWave: 0x21 } // Bb3 flat of B3 (8291 / 2 = 4145) flat index is A#3 (7825 / 2 = 3912)
    ];

    console.log("\n[TEST] Verifying notation parsing logic...");
    for (const testCase of testCases) {
        const result = await contract.parseNote.staticCall(testCase.note);
        const freq = Number(result.frequency);
        const wave = Number(result.waveform);

        console.log(`  Note "${testCase.note}" -> Freq: ${freq} (Expected: ${testCase.expectedFreq}), Wave: 0x${wave.toString(16)} (Expected: 0x${testCase.expectedWave.toString(16)})`);
        if (freq !== testCase.expectedFreq || wave !== testCase.expectedWave) {
            console.error(`  => FAIL: Validation failed for note ${testCase.note}`);
            process.exit(1);
        }
    }
    console.log("  => PASS: All note parsing validation cases matching expected frequencies and ADSR gate waveforms.");

    // 3. Benchmarks
    console.log("\n[BENCHMARK] Measuring gas used for parseNote...");
    const benchTx = await contract.parseNote("C4T", { gasLimit: 200000 });
    const benchReceipt = await benchTx.wait();
    console.log(`Gas used for parseNote transaction: ${benchReceipt.gasUsed.toString()} gas`);

    console.log("\n=== All Tests & Benchmarks Completed Successfully ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
