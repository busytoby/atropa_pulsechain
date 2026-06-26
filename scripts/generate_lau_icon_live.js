const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");

// RPC endpoints prioritizing local node, then public PulseChain mainnet
const RPC_URLS = [
    "http://127.0.0.1:8545",
    "https://rpc.pulsechain.com",
    "https://pulsechain.publicnode.com"
];

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
    console.log(`[PulseChain Linker] Falling back to offline address-hash mode.`);
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
    
    let activeProvider = null;
    let endpointUsed = "";
    
    for (const url of RPC_URLS) {
        console.log(`[PulseChain Linker] Checking RPC endpoint: ${url}...`);
        const provider = new ethers.JsonRpcProvider(url);
        const isOnline = await checkProviderConnection(provider);
        if (isOnline) {
            activeProvider = provider;
            endpointUsed = url;
            break;
        }
    }
    
    if (!activeProvider) {
        console.log(`[PulseChain Linker] All RPC endpoints unreachable.`);
        const cachePath = path.join(__dirname, "pulsechain_register_cache.json");
        if (fs.existsSync(cachePath)) {
            try {
                const cache = JSON.parse(fs.readFileSync(cachePath, "utf8"));
                const entry = cache[address];
                if (entry) {
                    console.log(`[PulseChain Linker] Found address register values in local cache.`);
                    const args = [
                        path.join(__dirname, "generate_lau_icon.py"),
                        address,
                        entry.r_base,
                        entry.r_channel,
                        entry.r_dynamo,
                        entry.r_foundation,
                        entry.c_base,
                        entry.c_channel,
                        entry.c_dynamo,
                        entry.c_foundation
                    ];
                    console.log(`[PulseChain Linker] Spawning Python renderer with cached values...`);
                    const child = spawn("python3", args);
                    child.stdout.on("data", (data) => process.stdout.write(data.toString()));
                    child.stderr.on("data", (data) => process.stderr.write(data.toString()));
                    child.on("close", (code) => {
                        if (code === 0) {
                            console.log(`[PulseChain Linker] Offline cached icon generation completed successfully.`);
                        } else {
                            console.error(`[PulseChain Linker] Python renderer exited with code ${code}`);
                        }
                    });
                    return;
                }
            } catch (e) {
                console.error(`[PulseChain Linker Error] Failed to read local cache: ${e.message}`);
            }
        }
        await runOfflineRenderer(address);
        return;
    }
    
    console.log(`[PulseChain Linker] Connected via ${endpointUsed}. Querying state for address: ${address}`);
    try {
        const lauArtifact = getContractArtifact("11_lau.sol", "LAU");
        const lau = new ethers.Contract(address, lauArtifact.abi, activeProvider);

        const on = await lau.On();
        const shaArtifact = getContractArtifact("02_sha.sol", "SHA");
        const sha = new ethers.Contract(on.Mu, shaArtifact.abi, activeProvider);

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

        // Cache the retrieved values
        const cachePath = path.join(__dirname, "pulsechain_register_cache.json");
        let cache = {};
        if (fs.existsSync(cachePath)) {
            try {
                cache = JSON.parse(fs.readFileSync(cachePath, "utf8"));
            } catch (e) {}
        }
        cache[address] = {
            r_base,
            r_channel,
            r_dynamo,
            r_foundation,
            c_base,
            c_channel,
            c_dynamo,
            c_foundation
        };
        try {
            fs.writeFileSync(cachePath, JSON.stringify(cache, null, 2), "utf8");
            console.log(`[PulseChain Linker] Updated local cache for address: ${address}`);
        } catch (e) {
            console.error(`[PulseChain Linker Error] Failed to write cache: ${e.message}`);
        }

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
