const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");
const fs = require("fs");

const BRIDGE_YUL_PATH = path.join(__dirname, "../scratch/cardco_bridge.yul");
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
    console.log("=== Starting Cardco Printer Bridge Unit Tests & Benchmarks ===");

    // Extract the Yul object into a temporary scratch file for compilation
    const srcYul = fs.readFileSync(path.join(__dirname, "../lore/cardco_peripheral_interfaces.md"), "utf8");
    const yulBlockStart = srcYul.indexOf('object "CardcoPrinterBridge" {');
    const yulBlockEnd = srcYul.indexOf('```', yulBlockStart);
    const yulCode = srcYul.substring(yulBlockStart, yulBlockEnd).trim();

    const scratchDir = path.join(__dirname, "../scratch");
    if (!fs.existsSync(scratchDir)) {
        fs.mkdirSync(scratchDir);
    }
    fs.writeFileSync(BRIDGE_YUL_PATH, yulCode);

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    let signers;
    try {
        signers = await provider.listAccounts();
    } catch (e) {
        console.error("Error connecting to local EVM node. Make sure Anvil or Hardhat is running!");
        process.exit(1);
    }
    const deployer = signers[0];

    // 1. Compile and deploy
    console.log("\n[DEPLOY] Compiling and deploying CardcoPrinterBridge.yul...");
    const bytecode = compileYul(BRIDGE_YUL_PATH);
    const tx = await deployer.sendTransaction({ data: bytecode, gasLimit: 4000000 });
    const receipt = await tx.wait();
    const contractAddress = receipt.contractAddress;
    console.log("Cardco Bridge contract deployed at:", contractAddress);

    const abi = [
        "function translateIecToCentronics(bytes serialPayload) public returns (bytes asciiOutput, uint256 strobeCount, uint256 cycles)",
        "function produceSpriteData(bytes asciiInput) public returns (bytes spriteOutput)"
    ];
    const contract = new ethers.Contract(contractAddress, abi, deployer);

    // 2. Run test cases
    // CBM PETSCII payload: "HELLO c64"
    // 'H' (72 / 0x48), 'E' (69 / 0x45), 'L' (76 / 0x4C), 'L' (76 / 0x4C), 'O' (79 / 0x4F), ' ' (32 / 0x20),
    // lowercase 'c' (0xC3 in PETSCII), '6' (54 / 0x36), '4' (52 / 0x34)
    const petsciiPayload = [0x48, 0x45, 0x4c, 0x4c, 0x4f, 0x20, 0xc3, 0x36, 0x34];
    const expectedAscii = "HELLO c64"; // 0xC3 maps to 0xC3 - 0x60 = 0x63 ('c')

    console.log("\n[TEST] Translating PETSCII payload:", JSON.stringify(petsciiPayload));
    const result = await contract.translateIecToCentronics.staticCall(Uint8Array.from(petsciiPayload));

    const asciiOutput = ethers.toUtf8String(result.asciiOutput);
    const strobeCount = Number(result.strobeCount);
    const cycles = Number(result.cycles);

    console.log(`  Translated output string: "${asciiOutput}"`);
    console.log(`  Centronics strobes:       ${strobeCount}`);
    console.log(`  Simulated CPU cycles:     ${cycles}`);

    if (asciiOutput === expectedAscii && strobeCount === petsciiPayload.length) {
        console.log("  => PASS: Cardco IEC-to-Centronics translation and PETSCII-to-ASCII transliteration verified!");
    } else {
        console.error(`  => FAIL: Translation output mismatch! Expected: "${expectedAscii}", Got: "${asciiOutput}"`);
        process.exit(1);
    }

    // 2.2 Test produceSpriteData
    const testString = "HELLO";
    console.log(`\n[TEST] Generating 8x8 sprite data for string: "${testString}"`);
    const spriteResultBytes = await contract.produceSpriteData.staticCall(ethers.toUtf8Bytes(testString));
    const spriteResult = ethers.getBytes(spriteResultBytes);
    
    console.log(`  Sprite data length: ${spriteResult.length} bytes (expected ${testString.length * 8})`);
    if (spriteResult.length !== testString.length * 8) {
        console.error("  => FAIL: Sprite data length mismatch!");
        process.exit(1);
    }
    
    // Draw character grids to console
    for (let charIdx = 0; charIdx < testString.length; charIdx++) {
        console.log(`\n  Sprite Grid for '${testString[charIdx]}':`);
        for (let row = 0; row < 8; row++) {
            const byteVal = spriteResult[charIdx * 8 + row];
            let rowString = "    ";
            for (let bit = 7; bit >= 0; bit--) {
                rowString += ((byteVal >> bit) & 1) ? "█" : "░";
            }
            console.log(rowString);
        }
    }
    console.log("  => PASS: Sprite generation validated!");

    // 3. Benchmarks
    console.log("\n[BENCHMARK] Measuring gas used for parallel print dispatch...");
    const benchTx = await contract.translateIecToCentronics(Uint8Array.from(petsciiPayload), { gasLimit: 200000 });
    const benchReceipt = await benchTx.wait();
    console.log(`Gas used for translateIecToCentronics transaction: ${benchReceipt.gasUsed.toString()} gas`);

    console.log("\n[BENCHMARK] Measuring gas used for sprite generation...");
    const spriteTx = await contract.produceSpriteData(ethers.toUtf8Bytes(testString), { gasLimit: 200000 });
    const spriteReceipt = await spriteTx.wait();
    console.log(`Gas used for produceSpriteData transaction: ${spriteReceipt.gasUsed.toString()} gas`);

    console.log("\n=== All Tests & Benchmarks Completed Successfully ===");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
