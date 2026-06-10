const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Front Panel Blinking Lights Monitor (lights.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/lights.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/lights.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const lightsABI = [
        "function updateBusState(uint256 addr, uint256 data, uint256 statusFlags) external returns (uint256)",
        "function getPanelLEDs() external view returns (uint256 addressLEDs, uint256 dataLEDs, uint256 statusLEDs, uint256[16] intensity)"
    ];

    const factory = new ethers.ContractFactory(lightsABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`BlinkingLightsPanel contract deployed at: ${contractAddr}\n`);

    // 1. Initial State Check
    console.log("Checking initial panel state...");
    let [addr, data, statusFlags, intensity] = await contract.getPanelLEDs();
    console.log(`Initial -> Addr: ${addr.toString()}, Data: ${data.toString()}, Status: ${statusFlags.toString()}`);
    if (Number(addr) !== 0 || Number(data) !== 0 || Number(statusFlags) !== 0) {
        console.error("FAIL: Initial states should be 0");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 2. Send updates
    // Update 1: Address = 1 (bit 0 = 1), Data = 0x55, Status = 1
    console.log("Sending Update 1 (Addr=1, Data=0x55, Status=1)...");
    let tx = await contract.updateBusState(1, 0x55, 1);
    await tx.wait();

    // Update 2: Address = 0 (bit 0 = 0), Data = 0xAA, Status = 2
    console.log("Sending Update 2 (Addr=0, Data=0xAA, Status=2)...");
    tx = await contract.updateBusState(0, 0xAA, 2);
    await tx.wait();

    // 3. Read back panel state
    console.log("Reading panel states after updates...");
    [addr, data, statusFlags, intensity] = await contract.getPanelLEDs();
    console.log(`Current -> Addr: ${addr.toString()}, Data: 0x${Number(data).toString(16)}, Status: ${statusFlags.toString()}`);
    
    // Intensity array: index 0 (bit 0) should have high intensity (255) due to 2 transitions out of 2 updates
    // index 1 (bit 1) should be 0 due to 0 transitions
    console.log(`Bit 0 intensity: ${intensity[0].toString()}`);
    console.log(`Bit 1 intensity: ${intensity[1].toString()}`);

    if (Number(addr) !== 0 || Number(data) !== 0xAA || Number(statusFlags) !== 2) {
        console.error("FAIL: Final states mismatched!");
        process.exit(1);
    }
    if (Number(intensity[0]) !== 255 || Number(intensity[1]) !== 0) {
        console.error("FAIL: Intensity calculations mismatched!");
        process.exit(1);
    }
    console.log("Passed!\n");

    console.log(">>> ALL FRONT PANEL LIGHTS TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
