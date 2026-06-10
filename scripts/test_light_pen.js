const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing GraphicsSystem Light Pen Simulation ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/graphicsSystem.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/graphicsSystem.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const gsABI = [
        "function triggerLightPen(uint256 x, uint256 y) external returns (uint256)",
        "function getLightPenState() external view returns (uint256 x, uint256 y)"
    ];

    const factory = new ethers.ContractFactory(gsABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`GraphicsSystem (Light Pen) deployed at: ${contractAddr}\n`);

    // Test Case 1: Initial state
    console.log("Reading initial Light Pen state...");
    let [x, y] = await contract.getLightPenState();
    console.log(`Initial coordinates -> X: ${x.toString()}, Y: ${y.toString()}`);
    if (Number(x) !== 0 || Number(y) !== 0) {
        console.error("FAIL: Initial coordinates should be 0, 0");
        process.exit(1);
    }
    console.log("Passed!\n");

    // Test Case 2: Triggering light pen at (140, 95)
    console.log("Triggering Light Pen at X=140, Y=95...");
    let tx = await contract.triggerLightPen(140, 95);
    await tx.wait();
    
    [x, y] = await contract.getLightPenState();
    console.log(`Retrieved coordinates -> X: ${x.toString()}, Y: ${y.toString()}`);
    if (Number(x) !== 140 || Number(y) !== 95) {
        console.error(`FAIL: Expected X=140, Y=95 but got X=${x}, Y=${y}`);
        process.exit(1);
    }
    console.log("Passed!\n");

    // Test Case 3: Triggering light pen at (312, 210)
    console.log("Triggering Light Pen at X=312, Y=210...");
    tx = await contract.triggerLightPen(312, 210);
    await tx.wait();
    
    [x, y] = await contract.getLightPenState();
    console.log(`Retrieved coordinates -> X: ${x.toString()}, Y: ${y.toString()}`);
    if (Number(x) !== 312 || Number(y) !== 210) {
        console.error(`FAIL: Expected X=312, Y=210 but got X=${x}, Y=${y}`);
        process.exit(1);
    }
    console.log("Passed!\n");

    console.log(">>> ALL GRAPHICS SYSTEM LIGHT PEN TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
