const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing TTL Pulse Catcher Logic Circuit (pulsecatcher.yul) ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling solidity/bin/pulsecatcher.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/pulsecatcher.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error("Compilation output did not contain 'Binary representation:'");
    }
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const pcABI = [
        "function initCircuit(uint256 triggerMode) external returns (uint256)",
        "function updateSignal(uint256 signalValue) external returns (uint256 latched, uint256 pulseLedState)",
        "function resetCircuit() external returns (uint256)"
    ];

    const factory = new ethers.ContractFactory(pcABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`TTLPulseCatcher contract deployed at: ${contractAddr}\n`);

    // 1. Initialize for Falling Edge (mode 0)
    console.log("Initializing circuit for Falling Edge (Mode 0)...");
    let tx = await contract.initCircuit(0, { gasLimit: 100000 });
    await tx.wait();

    // 2. Check initial signal inputs
    console.log("Updating signal to 0 (no transition)...");
    tx = await contract.updateSignal(0, { gasLimit: 100000 });
    await tx.wait();

    console.log("Updating signal to 1 (rising transition, mode is falling)...");
    tx = await contract.updateSignal(1, { gasLimit: 100000 });
    await tx.wait();

    console.log("Updating signal to 0 (falling transition 1 -> 0)...");
    tx = await contract.updateSignal(0, { gasLimit: 100000 });
    await tx.wait();
    console.log("Signal updated to 0 successfully.");

    // Check states
    // We can call getPanelLEDs or we can call updateSignal again to see the state returned or check latch directly?
    // Wait, let's call a view check to see if it's latched. Since updateSignal returns the state, we can simulate it with staticCall now
    let state = await contract.updateSignal.staticCall(0);
    console.log(`Latch State: ${state.latched.toString()}`);
    if (Number(state.latched) !== 1) {
        console.error("FAIL: Should be latched!");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 3. Verify it stays latched
    console.log("Updating signal to 1 (latch should persist)...");
    tx = await contract.updateSignal(1, { gasLimit: 100000 });
    await tx.wait();
    
    state = await contract.updateSignal.staticCall(1);
    console.log(`Latch: ${state.latched.toString()}`);
    if (Number(state.latched) !== 1) {
        console.error("FAIL: Latch state should persist.");
        process.exit(1);
    }
    console.log("Passed!\n");

    // 4. Reset circuit
    console.log("Resetting circuit...");
    tx = await contract.resetCircuit({ gasLimit: 100000 });
    await tx.wait();
    
    state = await contract.updateSignal.staticCall(1);
    console.log(`Latch: ${state.latched.toString()}`);
    if (Number(state.latched) !== 0) {
        console.error("FAIL: Latch state did not clear after reset.");
        process.exit(1);
    }
    console.log("Passed!\n");

    console.log(">>> ALL TTL PULSE CATCHER TESTS PASSED! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
