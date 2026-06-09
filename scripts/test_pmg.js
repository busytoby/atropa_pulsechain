const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Compile Yul helper
function compileYul(yulPath) {
    const absolutePath = path.resolve(__dirname, yulPath);
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation in solc output for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("=== STEP 1: Compiling and Deploying PmgSystem.yul ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const bytecode = compileYul("../solidity/bin/pmgSystem.yul");
    console.log("Compilation successful. Bytecode length:", bytecode.length);

    // Deploy contract directly
    const txDeploy = await deployer.sendTransaction({
        data: bytecode,
        gasLimit: 3000000
    });
    const receipt = await txDeploy.wait();
    const pmgAddress = receipt.contractAddress;
    console.log(`PmgSystem deployed at: ${pmgAddress}`);

    const pmgInterface = new ethers.Interface([
        "function updatePmg(uint8 isMissile, uint8 index, uint8 hpos, uint8 size, uint8 color) external returns (uint256)",
        "function getPmgState(uint8 isMissile, uint8 index) external view returns (uint8 hpos, uint8 size, uint8 color)",
        "function checkPmgCollisions() external returns (uint8)"
    ]);

    const pmgContract = new ethers.Contract(pmgAddress, pmgInterface, deployer);

    // Test 1: Set Player 0 state
    console.log("\n=== TEST 1: Registering Player 0 (HPOS=120, Size=1, Color=10) ===");
    const tx1 = await pmgContract.updatePmg(0, 0, 120, 1, 10);
    await tx1.wait();

    const stateP0 = await pmgContract.getPmgState(0, 0);
    console.log(`Player 0 State -> HPOS: ${stateP0[0]}, Size: ${stateP0[1]}, Color: ${stateP0[2]}`);
    if (Number(stateP0[0]) !== 120 || Number(stateP0[1]) !== 1 || Number(stateP0[2]) !== 10) {
        console.error("FAIL: Player 0 state mismatch!");
        process.exit(1);
    }
    console.log("SUCCESS: Player 0 registered correctly.");

    // Test 2: Set Player 1 far away (No collision)
    console.log("\n=== TEST 2: Registering Player 1 Far Away (HPOS=200, Size=0, Color=5) ===");
    const tx2 = await pmgContract.updatePmg(0, 1, 200, 0, 5);
    await tx2.wait();

    const collisionFar = await pmgContract.checkPmgCollisions.staticCall();
    console.log(`Collision Mask (Far): ${collisionFar} (Expected: 0)`);
    if (Number(collisionFar) !== 0) {
        console.error("FAIL: False collision detected!");
        process.exit(1);
    }
    console.log("SUCCESS: No false collision detected.");

    // Test 3: Move Player 1 to overlap (HPOS=130, Size 1 -> Player 0 is at 120 with size 1 (16px width), so overlaps)
    console.log("\n=== TEST 3: Moving Player 1 to overlap (HPOS=130) ===");
    const tx3 = await pmgContract.updatePmg(0, 1, 130, 0, 5);
    await tx3.wait();

    const collisionOverlap = await pmgContract.checkPmgCollisions.staticCall();
    console.log(`Collision Mask (Overlap): ${collisionOverlap} (Expected: 1)`);
    if (Number(collisionOverlap) !== 1) {
        console.error("FAIL: Failed to detect PMG horizontal overlap!");
        process.exit(1);
    }
    console.log("SUCCESS: Overlap collision correctly identified!");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
