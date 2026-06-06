const { ethers } = require("ethers");
const { execSync } = require("child_process");
const path = require("path");

const YUL_PATH = path.join(__dirname, "../solidity/bin/immutableFactory.yul");
const PROVIDER_URL = "http://127.0.0.1:8545";

async function main() {
    console.log("Compiling Yul contract...");
    let bytecode;
    try {
        const solcOutput = execSync(`solc --strict-assembly "${YUL_PATH}" --bin`, { encoding: "utf8" });
        const lines = solcOutput.split("\n");
        const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
        if (binIndex === -1) {
            throw new Error("Could not find binary representation");
        }
        bytecode = "0x" + lines[binIndex + 1].trim();
        console.log("Compiled successfully.");
    } catch (err) {
        console.error("Compilation failed:", err.message);
        process.exit(1);
    }

    console.log(`Connecting to EVM at ${PROVIDER_URL}...`);
    try {
        const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
        const signers = await provider.listAccounts();
        const deployer = signers[0];
        console.log(`Deploying from address: ${deployer.address}`);

        const tx = await deployer.sendTransaction({
            data: bytecode
        });
        const receipt = await tx.wait();
        console.log("\nDeployment Success!");
        console.log(`Factory Address: ${receipt.contractAddress}`);
    } catch (err) {
        console.error("Deployment failed. Is Anvil/EVM running?", err.message);
        process.exit(1);
    }
}

main();
