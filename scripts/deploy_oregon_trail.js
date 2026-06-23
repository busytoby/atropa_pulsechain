const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const PROVIDER_URL = "http://127.0.0.1:8545";
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

function getContractArtifact(filename, contractName) {
    const filePath = path.join(__dirname, `../Wallet/bin/Contracts/${filename}.json`);
    const fileContent = JSON.parse(fs.readFileSync(filePath, "utf8"));
    for (const key of Object.keys(fileContent.contracts)) {
        if (key.endsWith(`:${contractName}`)) {
            return fileContent.contracts[key];
        }
    }
    throw new Error(`Artifact for ${contractName} not found in ${filename}`);
}

async function main() {
    console.log("Connecting to local EVM...");
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];
    console.log(`Using address: ${deployer.address}`);

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const cpu6502Address = config.networks.localhost.cpu6502Address;
    if (!cpu6502Address) {
        console.error("cpu6502Address not found in localhost configuration.");
        process.exit(1);
    }

    console.log(`CPU6502 Address: ${cpu6502Address}`);
    const artifact = getContractArtifact("oregontrailtoken.sol", "OregonTrailToken");
    const factory = new ethers.ContractFactory(artifact.abi, artifact.bin, deployer);

    // Deploying with cpu address, memory address (e.g. 102), name, symbol
    console.log("Deploying OregonTrailToken...");
    const token = await factory.deploy(
        cpu6502Address,
        102,
        "Oregon Trail Token",
        "OTRT"
    );
    await token.waitForDeployment();
    const tokenAddress = await token.getAddress();
    console.log(` -> OregonTrailToken deployed at: ${tokenAddress}`);

    // Save to user_config.json
    config.networks.localhost.oregonTrailToken = tokenAddress;
    fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
    console.log("Saved oregonTrailToken address to config/user_config.json.");
}

main().catch(console.error);
