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
    const secondSigner = signers[1];
    console.log(`Deployer address: ${deployer.address}`);

    if (!fs.existsSync(CONFIG_PATH)) {
        console.error("Config not found.");
        process.exit(1);
    }
    const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
    const mariarahelLauAddress = config.networks.localhost.mariarahelLau;
    if (!mariarahelLauAddress) {
        console.error("mariarahelLau address not found in config.");
        process.exit(1);
    }

    console.log(`Testing LAU at address: ${mariarahelLauAddress}`);
    const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
    const lau = new ethers.Contract(mariarahelLauAddress, lauArtifact.abi, deployer);

    // 1. Read Type
    console.log("\n--- Testing constant variables/reads ---");
    const typeValue = await lau.Type();
    console.log(`Type(): ${typeValue}`);
    
    // 2. Read Username
    const username = await lau.Username();
    console.log(`Username(): ${username}`);

    // 3. Update Username
    console.log("\n--- Testing Username(string) write ---");
    const newUsername = "maria_test";
    const txUser = await lau["Username(string)"](newUsername);
    await txUser.wait();
    console.log(`Username updated. Read back Username(): ${await lau.Username()}`);

    // Revert it back to mariarahel
    await (await lau["Username(string)"]("mariarahel")).wait();

    // 4. Test Chat(string)
    console.log("\n--- Testing Chat(string) ---");
    const txChat = await lau.Chat("Hello world from local test!");
    await txChat.wait();
    console.log("Chat succeeded!");

    // 5. Test Alias(address, string) and Alias(address)
    console.log("\n--- Testing Alias(address, string) and Alias(address) ---");
    const dummyAddress = secondSigner.address;
    const txAlias = await lau["Alias(address,string)"](dummyAddress, "HelperSigner");
    await txAlias.wait();
    console.log("Alias set.");
    
    const readAlias = await lau["Alias(address)"](dummyAddress);
    console.log(`Read back Alias(address): ${readAlias}`);

    // 6. Test Leave()
    console.log("\n--- Testing Leave() ---");
    const txLeave = await lau.Leave();
    await txLeave.wait();
    const currentArea = await lau.CurrentArea();
    console.log(`CurrentArea after Leave(): ${currentArea}`);

    // 7. Test Void(bool, bool)
    console.log("\n--- Testing Void(bool, bool) ---");
    const txVoid = await lau.Void(true, true);
    await txVoid.wait();
    console.log("Void succeeded!");

    console.log("\n==================================================");
    console.log("LAU TEST SUCCESSFUL!");
    console.log("==================================================");
}

main().catch(console.error);
