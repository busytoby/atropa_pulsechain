const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("=== Testing Monte Carlo Policy Gradient Q-Learning on Folklore CPU ===");

    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("Compiling and deploying folklore.yul...");
    const yulPath = path.join(__dirname, "../solidity/bin/folklore.yul");
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    const bytecode = "0x" + lines[binIndex + 1].trim();

    const folkABI = [
        "function recordTransition(uint256 state, uint256 action) external returns (uint256)",
        "function applyMonteCarloScore(int256 score) external returns (uint256)",
        "function getPolicyWeight(uint256 state, uint256 action) external view returns (int256)"
    ];

    const factory = new ethers.ContractFactory(folkABI, bytecode, signer);
    const contract = await factory.deploy();
    await contract.waitForDeployment();
    const contractAddr = await contract.getAddress();
    console.log(`Folklore CPU deployed at: ${contractAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");
    async function sendTx(method, ...args) {
        const tx = await contract[method](...args, { nonce });
        nonce++;
        return tx.wait();
    }

    // Step 1: Query initial weights (should be 0)
    console.log("\nChecking initial state-action weights...");
    const initW1 = await contract.getPolicyWeight(12, 1);
    const initW2 = await contract.getPolicyWeight(34, 2);
    console.log(` -> Initial Weight (State 12, Action 1): ${initW1}`);
    console.log(` -> Initial Weight (State 34, Action 2): ${initW2}`);

    // Step 2: Record a sequence of transitions (State-Action pairs)
    console.log("\nRecording transition history for Episode 1...");
    await sendTx("recordTransition", 12, 1);
    await sendTx("recordTransition", 34, 2);
    await sendTx("recordTransition", 12, 3); // different action

    // Step 3: Apply a positive reward score (e.g. Win = +50 points)
    console.log("\nEpisode 1 Completed: WIN! Applying performance score +50...");
    await sendTx("applyMonteCarloScore", 50);

    // Step 4: Verify weights incremented
    const postW1 = await contract.getPolicyWeight(12, 1);
    const postW2 = await contract.getPolicyWeight(34, 2);
    const postW3 = await contract.getPolicyWeight(12, 3);
    const postW4 = await contract.getPolicyWeight(12, 2); // not visited
    console.log(` -> Post-Win Weight (State 12, Action 1): ${postW1}`);
    console.log(` -> Post-Win Weight (State 34, Action 2): ${postW2}`);
    console.log(` -> Post-Win Weight (State 12, Action 3): ${postW3}`);
    console.log(` -> Unvisited Weight (State 12, Action 2): ${postW4}`);

    // Step 5: Record another episode leading to loss (e.g. -20 points)
    console.log("\nRecording transition history for Episode 2...");
    await sendTx("recordTransition", 12, 1); // repeated visit
    await sendTx("recordTransition", 99, 4); // new visit

    console.log("Episode 2 Completed: LOSS! Applying performance score -20...");
    await sendTx("applyMonteCarloScore", -20);

    // Final checks
    const finalW1 = await contract.getPolicyWeight(12, 1);
    const finalW2 = await contract.getPolicyWeight(34, 2); // not visited in ep 2
    const finalW99 = await contract.getPolicyWeight(99, 4);

    console.log(` -> Final Weight (State 12, Action 1): ${finalW1} (Expected 30)`);
    console.log(` -> Final Weight (State 34, Action 2): ${finalW2} (Expected 50)`);
    console.log(` -> Final Weight (State 99, Action 4): ${finalW99} (Expected -20)`);

    const ok = finalW1 === 30n && finalW2 === 50n && finalW99 === -20n;
    if (ok) {
        console.log("\n>>> SUCCESS: Monte Carlo Policy Q-Learning verified perfectly on Folklore! <<<");
    } else {
        console.error("\n>>> FAILURE: Policy weights calculations mismatched! <<<");
        process.exit(1);
    }
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});
