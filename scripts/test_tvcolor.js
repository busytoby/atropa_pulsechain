const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing TV Color Graphics Signal Generator (tvcolor.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/tvcolor.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/tvcolor.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const tvABI = [
        "function generatePattern(uint256 patternType) external view returns (bytes colorSignals)"
    ];

    const factory = new ethers.ContractFactory(tvABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`TVColorGenerator contract deployed at: ${contractAddr}\n`);

    // Helper to get pixel values from byte array (each pixel is 3 bytes)
    function getPixel(bytesHex, pixelIndex) {
        const offset = 2 + pixelIndex * 6; // '0x' + index * 2 characters per byte * 3 bytes per pixel
        const yVal = parseInt(bytesHex.slice(offset, offset + 2), 16);
        const phaseVal = parseInt(bytesHex.slice(offset + 2, offset + 4), 16);
        const burstVal = parseInt(bytesHex.slice(offset + 4, offset + 6), 16);
        return { y: yVal, phase: phaseVal, burst: burstVal };
    }

    // 1. Test Pattern 0: Color Bars
    console.log("Generating Pattern 0 (Color Bars)...");
    const barsBytes = await contract.generatePattern(0);
    const barsLen = (barsBytes.length - 2) / 2;
    console.log(`Output size: ${barsLen} bytes (Expected: 3072)`);
    if (barsLen !== 3072) {
        console.error("FAIL: Incorrect color signals length.");
        process.exit(1);
    }

    // Pixel 0 (col 0): White bar (y: 255, phase: 0)
    let p0 = getPixel(barsBytes, 0);
    console.log(`Pixel 0 (White bar) -> Y: ${p0.y}, Phase: ${p0.phase}, Burst: ${p0.burst}`);
    if (p0.y !== 255 || p0.phase !== 0 || p0.burst !== 0) {
        console.error("FAIL: Mismatched White bar pixel parameters!");
        process.exit(1);
    }

    // Pixel 4 (col 4): Yellow bar (y: 180, phase: 135)
    let p4 = getPixel(barsBytes, 4);
    console.log(`Pixel 4 (Yellow bar) -> Y: ${p4.y}, Phase: ${p4.phase}, Burst: ${p4.burst}`);
    if (p4.y !== 180 || p4.phase !== 135 || p4.burst !== 0) {
        console.error("FAIL: Mismatched Yellow bar pixel parameters!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 2. Test Pattern 1: Rainbow sweep
    console.log("Generating Pattern 1 (Rainbow Sweep)...");
    const rainbowBytes = await contract.generatePattern(1);
    // Pixel 0 (col 0): phase = 0
    let rp0 = getPixel(rainbowBytes, 0);
    console.log(`Pixel 0 (Rainbow Start) -> Y: ${rp0.y}, Phase: ${rp0.phase}, Burst: ${rp0.burst}`);
    if (rp0.y !== 128 || rp0.phase !== 0 || rp0.burst !== 0) {
        console.error("FAIL: Rainbow start parameters mismatched!");
        process.exit(1);
    }

    // Pixel 16 (col 16): phase = (16 * 360 / 32) / 2 = 90
    let rp16 = getPixel(rainbowBytes, 16);
    console.log(`Pixel 16 (Rainbow Mid) -> Y: ${rp16.y}, Phase: ${rp16.phase}, Burst: ${rp16.burst}`);
    if (rp16.y !== 128 || rp16.phase !== 90 || rp16.burst !== 0) {
        console.error("FAIL: Rainbow mid-sweep parameters mismatched!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 3. Test Pattern 2: Grid with Burst
    console.log("Generating Pattern 2 (Grid with Burst)...");
    const gridBytes = await contract.generatePattern(2);
    // Pixel 0 (col 0): Y=0, Phase=180/2 = 90, Burst=1 (Color Burst active!)
    let gp0 = getPixel(gridBytes, 0);
    console.log(`Pixel 0 (Color Burst Active) -> Y: ${gp0.y}, Phase: ${gp0.phase}, Burst: ${gp0.burst}`);
    if (gp0.y !== 0 || gp0.phase !== 90 || gp0.burst !== 1) {
        console.error("FAIL: Color burst sync parameters mismatched!");
        process.exit(1);
    }

    // Pixel 2 (col 2, row 0): Y=255, Phase=0, Burst=0 (White grid horizontal line)
    let gp2 = getPixel(gridBytes, 2);
    console.log(`Pixel 2 (White Grid line) -> Y: ${gp2.y}, Phase: ${gp2.phase}, Burst: ${gp2.burst}`);
    if (gp2.y !== 255 || gp2.phase !== 0 || gp2.burst !== 0) {
        console.error("FAIL: Grid line parameters mismatched!");
        process.exit(1);
    }
    console.log("Passed!\n");

    console.log(">>> ALL TV COLOR GRAPHICS TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
