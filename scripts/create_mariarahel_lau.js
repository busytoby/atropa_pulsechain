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
    const lauFactoryAddress = config.networks.localhost.LAUFactory;
    if (!lauFactoryAddress) {
        console.error("LAUFactory address not found in localhost configuration.");
        process.exit(1);
    }

    console.log(`LAUFactory Address: ${lauFactoryAddress}`);
    const factoryArtifact = getContractArtifact("11c_laufactory.sol", "LAUFactory");
    const factoryContract = new ethers.Contract(lauFactoryAddress, factoryArtifact.abi, deployer);

    console.log("Creating new LAU User Token for 'mariarahel'...");
    const tx = await factoryContract.New("mariarahel", "MARIARAHEL");
    const receipt = await tx.wait();

    // NewLau event is emitted. Let's find it
    const newLauEvent = receipt.logs.find(x => x.fragment && x.fragment.name === "NewLau") || receipt.logs[0];
    const lauAddress = newLauEvent.args ? newLauEvent.args[1] : newLauEvent.address;
    console.log(` -> LAU deployed at: ${lauAddress}`);

    console.log("Setting username to 'mariarahel' on LAU contract...");
    const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
    const lauContract = new ethers.Contract(lauAddress, lauArtifact.abi, deployer);

    // Call Username function
    const txUsername = await lauContract["Username(string)"]("mariarahel");
    await txUsername.wait();
    console.log("Username successfully set!");

    // Save to user_config.json
    config.networks.localhost.mariarahelLau = lauAddress;
    fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2));
    console.log("Saved mariarahelLau address to config/user_config.json.");
}

main().catch(console.error);
