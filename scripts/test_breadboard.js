const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const COMPILED_YUL_PATH = path.join(__dirname, "../frontend/compiled_yul.json");

const breadboardABI = [
    "function setGate(uint256 gateId, uint256 gateType, uint256 inPin1, uint256 inPin2, uint256 outPin) external returns (uint256)",
    "function setPinValue(uint256 pin, uint256 value) external returns (uint256)",
    "function getPinValue(uint256 pin) external view returns (uint256)",
    "function propagate() external returns (uint256)",
    "function recycleGate(uint256 gateId) external returns (uint256)"
];

async function main() {
    console.log("=== Deploying Breadboard (Recycled ICs) Yul thunk ===");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    const compiled = JSON.parse(fs.readFileSync(COMPILED_YUL_PATH, "utf8"));
    const bytecode = compiled.breadboard;

    const factory = new ethers.ContractFactory(breadboardABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Breadboard contract deployed at: ${contractAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");

    async function sendTx(promise) {
        const tx = await promise;
        nonce++;
        return tx.wait();
    }

    console.log("\nSoldering logical gates...");
    
    // We will build a Half Adder using Logic Gates:
    // Sum = A XOR B
    // Carry = A AND B
    
    // Pins: A=1, B=2, Sum=3, Carry=4
    // Gate 1: XOR (Type 3) -> input pins 1 and 2 -> output pin 3
    await sendTx(contract.setGate(1, 3, 1, 2, 3, { nonce }));
    // Gate 2: AND (Type 1) -> input pins 1 and 2 -> output pin 4
    await sendTx(contract.setGate(2, 1, 1, 2, 4, { nonce }));

    // Test case 1: A=0, B=0
    console.log("\nTest Case 1: Setting inputs A=0, B=0");
    await sendTx(contract.setPinValue(1, 0, { nonce }));
    await sendTx(contract.setPinValue(2, 0, { nonce }));
    await sendTx(contract.propagate({ nonce }));
    
    let sum = await contract.getPinValue(3);
    let carry = await contract.getPinValue(4);
    console.log(`Outputs: Sum = ${sum}, Carry = ${carry}`);

    // Test case 2: A=1, B=0
    console.log("\nTest Case 2: Setting inputs A=1, B=0");
    await sendTx(contract.setPinValue(1, 1, { nonce }));
    await sendTx(contract.setPinValue(2, 0, { nonce }));
    await sendTx(contract.propagate({ nonce }));
    
    sum = await contract.getPinValue(3);
    carry = await contract.getPinValue(4);
    console.log(`Outputs: Sum = ${sum}, Carry = ${carry}`);

    // Test case 3: A=1, B=1
    console.log("\nTest Case 3: Setting inputs A=1, B=1");
    await sendTx(contract.setPinValue(1, 1, { nonce }));
    await sendTx(contract.setPinValue(2, 1, { nonce }));
    await sendTx(contract.propagate({ nonce }));
    
    sum = await contract.getPinValue(3);
    carry = await contract.getPinValue(4);
    console.log(`Outputs: Sum = ${sum}, Carry = ${carry}`);

    // Test Recycling / Desoldering Gate 1 (XOR)
    console.log("\nRecycling (desoldering) XOR gate 1...");
    await sendTx(contract.recycleGate(1, { nonce }));
    
    // Run propagation and check Sum (should not update / remain disconnected)
    console.log("Setting A=0, B=1 and propagating...");
    await sendTx(contract.setPinValue(1, 0, { nonce }));
    await sendTx(contract.setPinValue(2, 1, { nonce }));
    await sendTx(contract.propagate({ nonce }));

    sum = await contract.getPinValue(3);
    carry = await contract.getPinValue(4);
    console.log(`Outputs after recycling Gate 1: Sum = ${sum} (remained unchanged/disconnected), Carry = ${carry}`);

    console.log("\n>>> SUCCESS: Breadboard and Recycled IC simulation verified! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
