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
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const mariarahelLauAddress = config.networks.localhost.mariarahelLau;

    const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
    const lau = new ethers.Contract(mariarahelLauAddress, lauArtifact.abi, deployer);

    // Saat array indices: 0 -> Pole/Identity, 1 -> Soul, 2 -> Aura
    const soul = await lau.Saat(1);
    const aura = await lau.Saat(2);
    const identity = await lau.Saat(0);

    console.log("\n==================================================");
    console.log("mariarahel LAU METRICS:");
    console.log("==================================================");
    console.log(`Identity: ${identity.toString()}`);
    console.log(`Soul:     ${soul.toString()}`);
    console.log(`Aura:     ${aura.toString()}`);
    console.log("==================================================");
}

main().catch(console.error);
