const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

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

// Timeout helper for provider connection
async function checkProviderConnection(provider, timeoutMs = 2500) {
    return new Promise((resolve) => {
        const timer = setTimeout(() => {
            resolve(false);
        }, timeoutMs);
        
        provider.getNetwork()
            .then(() => {
                clearTimeout(timer);
                resolve(true);
            })
            .catch(() => {
                clearTimeout(timer);
                resolve(false);
            });
    });
}

async function runOfflineRenderer(address) {
    console.log(`[PulseChain Linker] RPC unreachable. Falling back to offline address-hash mode.`);
    const pyScript = path.join(__dirname, "generate_lau_icon.py");
    const child = spawn("python3", [pyScript, address]);

    child.stdout.on("data", (data) => process.stdout.write(data.toString()));
    child.stderr.on("data", (data) => process.stderr.write(data.toString()));
    child.on("close", (code) => {
        if (code === 0) {
            console.log(`[PulseChain Linker] Offline icon generation completed successfully.`);
        } else {
            console.error(`[PulseChain Linker] Python renderer exited with code ${code}`);
        }
    });
}

async function main() {
    let address = process.argv[2];
    if (!address) {
        try {
            const configPath = path.join(__dirname, "../config/user_config.json");
            const config = JSON.parse(fs.readFileSync(configPath, "utf8"));
            address = config.networks.localhost.mariarahelLau;
        } catch (e) {
            console.error("Usage: node generate_lau_icon_live.js <LAU_ADDRESS>");
            process.exit(1);
        }
    }
    
    address = address.toLowerCase().trim();
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    
    console.log(`[PulseChain Linker] Attempting connection to local RPC node at ${PROVIDER_URL}...`);
    const isOnline = await checkProviderConnection(provider);
    
    if (!isOnline) {
        await runOfflineRenderer(address);
        return;
    }
    
    console.log(`[PulseChain Linker] RPC connected. Querying contract state for address: ${address}`);
    try {
        const signers = await provider.listAccounts();
        const deployer = signers[0];

        const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
        const lau = new ethers.Contract(address, lauArtifact.abi, deployer);

        const on = await lau.On();
        const shaArtifact = getContractArtifact("02_sha.sol", "SHA");
        const sha = new ethers.Contract(on.Mu, shaArtifact.abi, deployer);

        const fa = await sha.View();

        const r_base = fa.Base.toString();
        const r_channel = fa.Channel.toString();
        const r_dynamo = fa.Monopole.toString();
        const r_foundation = fa.Foundation.toString();

        const c_base = on.Xi.toString();
        const c_channel = on.Ring.toString();
        const c_dynamo = on.Omicron.toString();
        const c_foundation = on.Omega.toString();

        console.log(`[PulseChain Linker] Successfully resolved live register values.`);
        console.log(`- Rod [Base: ${r_base}, Channel: ${r_channel}, Dynamo: ${r_dynamo}, Foundation: ${r_foundation}]`);
        console.log(`- Cone [Base: ${c_base}, Channel: ${c_channel}, Dynamo: ${c_dynamo}, Foundation: ${c_foundation}]`);

        const pyScript = path.join(__dirname, "generate_lau_icon.py");
        const args = [
            pyScript,
            address,
            r_base,
            r_channel,
            r_dynamo,
            r_foundation,
            c_base,
            c_channel,
            c_dynamo,
            c_foundation
        ];

        console.log(`[PulseChain Linker] Spawning Python renderer...`);
        const child = spawn("python3", args);

        child.stdout.on("data", (data) => process.stdout.write(data.toString()));
        child.stderr.on("data", (data) => process.stderr.write(data.toString()));
        child.on("close", (code) => {
            if (code === 0) {
                console.log(`[PulseChain Linker] Live icon generation completed successfully.`);
            } else {
                console.error(`[PulseChain Linker] Python renderer exited with code ${code}`);
            }
        });
    } catch (err) {
        console.error(`[PulseChain Linker Error] Failed to resolve chain states: ${err.message}`);
        await runOfflineRenderer(address);
    }
}

main().catch(console.error);
