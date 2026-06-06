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
    throw new Error(`Artifact for ${contractName} not found`);
}

async function main() {
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const deployer = signers[0];

    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const mariarahelLauAddress = config.networks.localhost.mariarahelLau;

    // Load LAU contract
    const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
    const lau = new ethers.Contract(mariarahelLauAddress, lauArtifact.abi, deployer);

    // Read On (Bao struct)
    console.log("Reading Bao (On) properties from LAU...");
    const on = await lau.On();
    
    console.log("\n==================================================");
    console.log("BAO STRUCT VALUES (On):");
    console.log("==================================================");
    console.log(`Phi (LAU address):  ${on.Phi}`);
    console.log(`SHA contract (Mu):  ${on.Mu}`);
    console.log(`Xi:                 ${on.Xi.toString()}`);
    console.log(`Pi:                 ${on.Pi.toString()}`);
    console.log(`Shio contract:      ${on.Shio}`);
    console.log(`Ring:               ${on.Ring.toString()}`);
    console.log(`Omicron:            ${on.Omicron.toString()}`);
    console.log(`Omega:              ${on.Omega.toString()}`);
    console.log("==================================================");

    // Load SHA contract
    console.log("\nQuerying SHA contract (Mu) properties...");
    const shaArtifact = getContractArtifact("02_sha.sol", "SHA");
    const sha = new ethers.Contract(on.Mu, shaArtifact.abi, deployer);

    // Call View() to read Fa struct properties
    const fa = await sha.View();

    console.log("\n==================================================");
    console.log("SHA FA STRUCT VALUES (Mu.View()):");
    console.log("==================================================");
    console.log(`Base:         ${fa.Base.toString()}`);
    console.log(`Secret:       ${fa.Secret.toString()}`);
    console.log(`Signal:       ${fa.Signal.toString()}`);
    console.log(`Channel:      ${fa.Channel.toString()}`);
    console.log(`Contour:      ${fa.Contour.toString()}`);
    console.log(`Pole:         ${fa.Pole.toString()}`);
    console.log(`Identity:     ${fa.Identity.toString()}`);
    console.log(`Foundation:   ${fa.Foundation.toString()}`);
    console.log(`Element:      ${fa.Element.toString()}`);
    console.log(`Coordinate:   ${fa.Coordinate.toString()}`);
    console.log(`Charge:       ${fa.Charge.toString()}`);
    console.log(`Chin:         ${fa.Chin.toString()}`);
    console.log(`Monopole:     ${fa.Monopole.toString()}`);
    console.log("==================================================");
}

main().catch(console.error);
