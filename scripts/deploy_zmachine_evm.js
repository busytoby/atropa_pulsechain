const { ethers } = require("ethers");
const { execSync } = require("child_process");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const ADDRESSES_PATH = path.join(__dirname, "deployed_addresses_localhost.json");

function compileYul(yulPath) {
    const absolutePath = path.resolve(__dirname, yulPath);
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log(`Using address: ${deployer.address}`);

    // Compile zmachine
    console.log("Compiling zmachine.yul...");
    const zmBytecode = compileYul("../solidity/bin/zmachine.yul");
    const zmFactory = new ethers.ContractFactory([], zmBytecode, deployer);
    
    console.log("Deploying zmachine directly to Anvil...");
    const zmContract = await zmFactory.deploy();
    await zmContract.waitForDeployment();
    const zmAddress = await zmContract.getAddress();
    console.log(`zmachine deployed at: ${zmAddress}`);

    // Compile zmachineParser
    console.log("Compiling zmachineParser.yul...");
    const parserBytecode = compileYul("../solidity/bin/zmachineParser.yul");
    const parserFactory = new ethers.ContractFactory([], parserBytecode, deployer);

    console.log("Deploying zmachineParser directly to Anvil (this may use substantial gas)...");
    const parserContract = await parserFactory.deploy();
    await parserContract.waitForDeployment();
    const parserAddress = await parserContract.getAddress();
    console.log(`zmachineParser deployed at: ${parserAddress}`);

    // Save to deployed_addresses_localhost.json
    const deployed = {
        zmachine: zmAddress,
        zmachineParser: parserAddress
    };
    fs.writeFileSync(ADDRESSES_PATH, JSON.stringify(deployed, null, 2));
    console.log(`Saved addresses to ${ADDRESSES_PATH}`);

    // Link parser to zmachine
    console.log("Binding Parser address to Z-Machine...");
    const zmBindContract = new ethers.Contract(zmAddress, [
        "function bindParserAddress(address parser) public returns (bool)"
    ], deployer);
    await (await zmBindContract.bindParserAddress(parserAddress)).wait();
    console.log("Link complete.");
}

main().catch(console.error);
