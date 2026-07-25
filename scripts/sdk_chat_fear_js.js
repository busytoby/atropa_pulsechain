const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");

const userConfig = JSON.parse(fs.readFileSync(path.join(__dirname, "../config/user_config.json"), "utf8"));
const localhost = userConfig.networks.localhost;

const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");

function getContractArtifact(fileName, contractName) {
    const filePath = path.join(__dirname, `../Wallet/bin/Contracts/${fileName}.json`);
    const fileContent = fs.readFileSync(filePath, "utf8");
    const json = JSON.parse(fileContent);
    return json.contracts[`dysnomia/${fileName}:${contractName}`] || json.contracts[`dysnomia/domain/sky/${fileName}:${contractName}`] || json.contracts[`dysnomia/domain/dan/${fileName}:${contractName}`];
}

async function main() {
    const deployer = await provider.getSigner();
    
    // Address of qFEAR contract
    const qFearAddress = "0xD2e813C297f3cf9985bAa5331CEC80778755BF2a";
    
    console.log("Transacting directly to speak a word sentence to the qFEAR qing chat using CHOA...");
    
    const choaContract = new ethers.Contract(
        localhost.CHOA,
        getContractArtifact("02_choa.sol", "CHOA").abi,
        deployer
    );

    // Call CHOA.Chat on the qFEAR Qing address
    const message = "Auncient Fear Word Sentence";
    const tx = await choaContract.Chat(qFearAddress, message);
    const receipt = await tx.wait();
    
    console.log("Transaction successfully completed!");
    console.log(" -> Gas Used:", receipt.gasUsed.toString());
    console.log(" -> Transaction Hash:", receipt.hash);
}

main().catch(console.error);
