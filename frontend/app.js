const FACTORY_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
const PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B";

const FACTORY_ABI = [
    "function resolve(bytes32 salt, bytes32 bytecodeHash) external view returns (address)"
];

const PKMINTER_ABI = [
    "function New(string memory Name, string memory Symbol, uint8 Complexity, address[] memory Signers, bytes32 Nonce, bytes[] memory Signatures) public returns (address)"
];

let provider;
let signer;
let privateKeys = [];
let walletAddresses = [];

// DOM Elements
const btnConnect = document.getElementById("btnConnect");
const networkBadge = document.getElementById("networkBadge");
const consoleLogs = document.getElementById("consoleLogs");
const btnClearLogs = document.getElementById("btnClearLogs");
const keysList = document.getElementById("keysList");

// Yul Factory DOM
const create2Salt = document.getElementById("create2Salt");
const create2Bytecode = document.getElementById("create2Bytecode");
const btnPredict = document.getElementById("btnPredict");
const btnDeployFactory = document.getElementById("btnDeployFactory");
const factoryResultBox = document.getElementById("factoryResultBox");
const predictedAddrVal = document.getElementById("predictedAddrVal");

// PKMinter DOM
const tokenName = document.getElementById("tokenName");
const tokenSymbol = document.getElementById("tokenSymbol");
const ttNonce = document.getElementById("ttNonce");
const btnGenNonce = document.getElementById("btnGenNonce");
const btnDeployTT = document.getElementById("btnDeployTT");

// Setup Console Logging
function log(msg, type = "info") {
    const time = new Date().toLocaleTimeString();
    const line = document.createElement("div");
    line.className = `log-line ${type}`;
    line.innerText = `[${time}] ${msg}`;
    consoleLogs.appendChild(line);
    consoleLogs.scrollTop = consoleLogs.scrollHeight;
}

btnClearLogs.addEventListener("click", () => {
    consoleLogs.innerHTML = "";
    log("Console cleared.");
});

// Load Config Keys from Local Server
async function loadConfigKeys() {
    try {
        const res = await fetch("/api/config");
        if (!res.ok) throw new Error("Failed to fetch config");
        const config = await res.json();
        
        privateKeys = config.default.keys || [];
        keysList.innerHTML = "";
        walletAddresses = [];

        privateKeys.forEach((key, index) => {
            const wallet = new ethers.Wallet(key);
            walletAddresses.push(wallet.address);
            
            const item = document.createElement("div");
            item.className = "key-item";
            item.innerHTML = `
                <div class="key-index">Key #${index + 1}</div>
                <div class="key-address">Address: ${wallet.address}</div>
                <div class="key-privkey">Private: ${key.substring(0, 8)}...${key.substring(key.length - 8)}</div>
            `;
            keysList.appendChild(item);
        });
        
        log(`Loaded ${privateKeys.length} keys from user_config.json successfully.`, "success");
    } catch (err) {
        log(`Could not load local keys: ${err.message}. Make sure server.js is running.`, "error");
    }
}

// Connect Wallet
async function connectWallet() {
    if (typeof window.ethereum === "undefined") {
        log("No Web3 provider detected! Please install MetaMask or another Web3 wallet.", "error");
        return;
    }

    try {
        log("Requesting account access...");
        provider = new ethers.BrowserProvider(window.ethereum);
        signer = await provider.getSigner();
        const address = await signer.getAddress();
        
        btnConnect.innerText = `${address.substring(0, 6)}...${address.substring(address.length - 4)}`;
        log(`Wallet connected: ${address}`, "success");

        // Fetch network details
        const network = await provider.getNetwork();
        let networkName = `Chain ID: ${network.chainId}`;
        if (network.chainId === 31337n) {
            networkName = "Localhost (Anvil)";
        } else if (network.chainId === 369n) {
            networkName = "PulseChain Mainnet";
        }
        
        networkBadge.innerText = networkName;
        networkBadge.className = "network-badge connected";
        log(`Active network: ${networkName} (Chain ID: ${network.chainId})`, "info");
    } catch (err) {
        log(`Connection failed: ${err.message}`, "error");
    }
}

let config = {};

// Load Config Keys from Local Server
async function loadConfigKeys() {
    try {
        const res = await fetch("/api/config");
        if (!res.ok) throw new Error("Failed to fetch config");
        config = await res.json();
        
        privateKeys = config.default.keys || [];
        keysList.innerHTML = "";
        walletAddresses = [];

        privateKeys.forEach((key, index) => {
            const wallet = new ethers.Wallet(key);
            walletAddresses.push(wallet.address);
            
            const item = document.createElement("div");
            item.className = "key-item";
            item.innerHTML = `
                <div class="key-index">Key #${index + 1}</div>
                <div class="key-address">Address: ${wallet.address}</div>
                <div class="key-privkey">Private: ${key.substring(0, 8)}...${key.substring(key.length - 8)}</div>
            `;
            keysList.appendChild(item);
        });
        
        log(`Loaded ${privateKeys.length} keys from user_config.json successfully.`, "success");
    } catch (err) {
        log(`Could not load local keys: ${err.message}. Make sure server.js is running.`, "error");
    }
}

btnConnect.addEventListener("click", connectWallet);

// Generate Nonce helper
btnGenNonce.addEventListener("click", () => {
    const nonce = ethers.hexlify(ethers.randomBytes(32));
    ttNonce.value = nonce;
    log(`Generated new nonce: ${nonce}`, "info");
});

// Initialize form defaults
create2Salt.value = ethers.hexlify(ethers.randomBytes(32));
ttNonce.value = ethers.hexlify(ethers.randomBytes(32));

// Predict CREATE2 Address
btnPredict.addEventListener("click", async () => {
    if (!provider) {
        log("Connect your wallet first.", "warning");
        return;
    }

    const salt = create2Salt.value.trim();
    let bytecode = create2Bytecode.value.trim() || create2Bytecode.placeholder;

    if (!ethers.isHexString(salt) || salt.length !== 66) {
        log("Invalid salt format (must be 32-byte hex).", "error");
        return;
    }

    const bytecodeHash = ethers.keccak256(bytecode);
    log(`Computing CREATE2 prediction for hash: ${bytecodeHash}...`);

    try {
        const iface = new ethers.Interface(FACTORY_ABI);
        const calldata = iface.encodeFunctionData("resolve", [salt, bytecodeHash]);
        const predictedRaw = await provider.call({
            to: FACTORY_ADDRESS,
            data: calldata
        });
        
        if (predictedRaw === "0x") {
            throw new Error("Contract returned no data. Check that your browser wallet is connected to the Local Anvil node (http://127.0.0.1:8545) and the factory is deployed.");
        }
        
        const predicted = ethers.getAddress(predictedRaw);
        predictedAddrVal.innerText = predicted;
        factoryResultBox.classList.remove("hidden");
        log(`Predicted Address: ${predicted}`, "success");
    } catch (err) {
        log(`Prediction failed: ${err.message}`, "error");
    }
});

// Deploy via Factory
btnDeployFactory.addEventListener("click", async () => {
    if (!signer) {
        log("Connect your wallet first.", "warning");
        return;
    }

    const salt = create2Salt.value.trim();
    let bytecode = create2Bytecode.value.trim() || create2Bytecode.placeholder;

    log("Broadcasting deployment via Yul factory...");
    try {
        // Since we are interacting via MetaMask, we'll manually send the transaction
        // utilizing createAuthorized (selector 0xb5ba0c68)
        // Wait, for custom deployment, we can use createAuthorized if we sign it, or executeDeployment.
        // Let's use executeDeployment (selector 0x93d9b8f0) since it's simpler and doesn't require signatures!
        // executeDeployment(bytes bytecode, bytes32 salt)
        const iface = new ethers.Interface([
            "function executeDeployment(bytes bytecode, bytes32 salt) external returns (address)"
        ]);
        const calldata = iface.encodeFunctionData("executeDeployment", [bytecode, salt]);

        log("Sending transaction...");
        const tx = await signer.sendTransaction({
            to: FACTORY_ADDRESS,
            data: calldata
        });
        
        log(`Transaction sent. Hash: ${tx.hash}. Waiting for confirmation...`, "info");
        const receipt = await tx.wait();
        log(`Confirmed in block ${receipt.blockNumber}! Status: ${receipt.status === 1 ? "SUCCESS" : "FAILED"}`, "success");
    } catch (err) {
        log(`Deployment failed: ${err.message}`, "error");
    }
});

// Listen for Key Source Mode dropdown changes to toggle text areas
const keySourceMode = document.getElementById("keySourceMode");
const customKeysContainer = document.getElementById("customKeysContainer");
const customKeysInput = document.getElementById("customKeysInput");
const tokenComplexity = document.getElementById("tokenComplexity");

keySourceMode.addEventListener("change", () => {
    if (keySourceMode.value === "provided") {
        customKeysContainer.classList.remove("hidden");
    } else {
        customKeysContainer.classList.add("hidden");
    }
});

// Deploy TT Token via PKMinter
btnDeployTT.addEventListener("click", async () => {
    const name = tokenName.value.trim();
    const symbol = tokenSymbol.value.trim();
    const nonce = ttNonce.value.trim();
    const complexity = parseInt(tokenComplexity.value) || 1;

    if (!ethers.isHexString(nonce) || nonce.length !== 66) {
        log("Invalid nonce format (must be 32-byte hex).", "error");
        return;
    }

    let activeKeys = [];
    let activeAddresses = [];

    if (keySourceMode.value === "config") {
        if (privateKeys.length < complexity) {
            log(`Loaded config only has ${privateKeys.length} keys, but you requested complexity ${complexity}!`, "error");
            return;
        }
        activeKeys = privateKeys.slice(0, complexity);
    } else if (keySourceMode.value === "random") {
        log(`Generating ${complexity} random keys/wallets for signing...`);
        for (let i = 0; i < complexity; i++) {
            const wallet = ethers.Wallet.createRandom();
            activeKeys.push(wallet.privateKey);
        }
    } else if (keySourceMode.value === "provided") {
        const rawKeys = customKeysInput.value.split(",")
            .map(k => k.trim())
            .filter(k => k.length > 0);
        if (rawKeys.length < complexity) {
            log(`You must provide at least ${complexity} private keys (currently provided: ${rawKeys.length})!`, "error");
            return;
        }
        activeKeys = rawKeys.slice(0, complexity);
    }

    // Convert private keys to matching addresses
    activeKeys.forEach((key, index) => {
        try {
            const wallet = new ethers.Wallet(key);
            activeAddresses.push(wallet.address);
        } catch (e) {
            log(`Invalid private key at position ${index + 1}: ${e.message}`, "error");
        }
    });

    if (activeAddresses.length !== complexity) {
        log("Error validating key mapping. Ensure key list is accurate.", "error");
        return;
    }

    log(`Signing nonce: ${nonce} with ${complexity} keys...`);
    const signatures = [];
    try {
        activeKeys.forEach((key, index) => {
            const wallet = new ethers.Wallet(key);
            const sig = wallet.signingKey.sign(nonce);
            const flatSig = ethers.concat([sig.r, sig.s, ethers.toBeHex(sig.v)]);
            signatures.push(flatSig);
        });
        log(`All ${complexity} signatures successfully generated.`, "success");
    } catch (err) {
        log(`Signing failed: ${err.message}`, "error");
        return;
    }

    log("Preparing PKMinter.New() transaction...");
    try {
        let activeSigner = signer;
        if (!activeSigner) {
            log("No browser wallet connected. Falling back to local deployer wallet...", "info");
            const localProvider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            const deployerPrivateKey = "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d";
            activeSigner = new ethers.Wallet(deployerPrivateKey, localProvider);
        }

        const pkMinter = new ethers.Contract(PKMINTER_ADDRESS, PKMINTER_ABI, activeSigner);
        
        log("Sending transaction to PKMinter...");
        const tx = await pkMinter.New(
            name,
            symbol,
            complexity,
            activeAddresses,
            nonce,
            signatures
        );

        log(`Transaction sent. Hash: ${tx.hash}. Waiting for confirmation...`, "info");
        const receipt = await tx.wait();
        log(`TT Token Deployed! Confirmed in block ${receipt.blockNumber}. Status: ${receipt.status === 1 ? "SUCCESS" : "FAILED"}`, "success");
        
        // Save dynamically generated keys to the local server configuration relative to the address
        if (keySourceMode.value === "random") {
            log("Saving generated keys locally inside config/user_config.json...");
            let deployedAddress = null;
            if (receipt.logs && receipt.logs.length > 0) {
                for (const rLog of receipt.logs) {
                    if (rLog.address && rLog.address.toLowerCase() !== PKMINTER_ADDRESS.toLowerCase()) {
                        deployedAddress = rLog.address;
                    }
                }
            }
            if (!deployedAddress) {
                deployedAddress = ethers.getCreate2Address(PKMINTER_ADDRESS, nonce, ethers.keccak256("0x"));
            }
            
            const saveRes = await fetch("/api/save-keys", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    address: deployedAddress,
                    keys: activeKeys,
                    name: name
                })
            });

            if (saveRes.ok) {
                log(`Successfully saved keys for ${name} (${deployedAddress}) in config.`, "success");
                loadConfigKeys();
            }
        }
    } catch (err) {
        log(`PKMinter deployment failed: ${err.message}`, "error");
    }
});

// Genesis launch handler
const btnLaunchGenesis = document.getElementById("btnLaunchGenesis");
const genesisComplexity = document.getElementById("genesisComplexity");
const genesisSalt = document.getElementById("genesisSalt");
const genesisName = document.getElementById("genesisName");
const genesisSymbol = document.getElementById("genesisSymbol");

// Populate initial genesis salt
genesisSalt.value = ethers.hexlify(ethers.randomBytes(32));

btnLaunchGenesis.addEventListener("click", async () => {
    const complexity = parseInt(genesisComplexity.value) || 1;
    const salt = genesisSalt.value.trim();
    const tokenNameStr = genesisName.value.trim();
    const tokenSymbolStr = genesisSymbol.value.trim();

    if (!ethers.isHexString(salt) || salt.length !== 66) {
        log("Genesis salt format is invalid (must be 32-byte hex).", "error");
        return;
    }

    log(`🌌 Starting Genesis Launch (Complexity: ${complexity})...`);
    
    // 1. Generate new random keys and signers
    const generatedKeys = [];
    const generatedAddresses = [];
    log(`Generating ${complexity} new random private keys for Genesis token signing...`);
    for (let i = 0; i < complexity; i++) {
        const wallet = ethers.Wallet.createRandom();
        generatedKeys.push(wallet.privateKey);
        generatedAddresses.push(wallet.address);
    }

    // 2. We use the salt as the signing nonce. Sign it with all complexity-sized keys.
    log(`Signing genesis nonce/salt: ${salt}...`);
    const signatures = [];
    try {
        generatedKeys.forEach((key) => {
            const wallet = new ethers.Wallet(key);
            const sig = wallet.signingKey.sign(salt);
            const flatSig = ethers.concat([sig.r, sig.s, ethers.toBeHex(sig.v)]);
            signatures.push(flatSig);
        });
        log("Generated all signatures successfully.", "success");
    } catch (e) {
        log(`Failed generating signatures: ${e.message}`, "error");
        return;
    }

    // 3. Predict final address using PKMinter's deployed salt predictability or standard CREATE2 logic.
    log("Invoking PKMinter.New()...");
    try {
        let activeSigner = signer;
        if (!activeSigner) {
            log("No browser wallet connected. Falling back to local deployer wallet...", "info");
            const localProvider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            // Use the standard Anvil account #0 private key
            const deployerPrivateKey = "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d";
            activeSigner = new ethers.Wallet(deployerPrivateKey, localProvider);
        }

        const pkMinter = new ethers.Contract(PKMINTER_ADDRESS, PKMINTER_ABI, activeSigner);
        const tx = await pkMinter.New(
            tokenNameStr,
            tokenSymbolStr,
            complexity,
            generatedAddresses,
            salt,
            signatures
        );
        log(`Transaction submitted: ${tx.hash}. Waiting for confirmation...`, "info");
        const receipt = await tx.wait();

        // 4. Extract token deployment address from transaction events
        // New signature event topic or return address: let's query the receipt logs or compute predicted address
        // The transaction receipt has logs, or we can read the contract events.
        // Let's look for address in the log receipt or try to compute it.
        // Alternatively, the PKMinter.New method returns the address, but since it's a write tx, we extract from logs.
        let deployedAddress = null;
        if (receipt.logs && receipt.logs.length > 0) {
            // Usually the last log or a log containing address
            // Standard ERC20 deploy logs or custom log topics.
            // Let's fall back to checking log topics or computing prediction,
            // or simply reading target addresses. Let's trace it.
            // We can scan the receipt for contract address creation, or log addresses.
            for (const rLog of receipt.logs) {
                if (rLog.address && rLog.address.toLowerCase() !== PKMINTER_ADDRESS.toLowerCase()) {
                    deployedAddress = rLog.address;
                }
            }
        }

        if (!deployedAddress) {
            // If log trace failed, fallback to querying the config networks or warning
            log("Genesis contract deployed successfully but target address could not be parsed automatically from logs.", "warning");
            deployedAddress = ethers.getCreate2Address(PKMINTER_ADDRESS, salt, ethers.keccak256("0x")); // mock fallback estimation
        }

        log(`🌌 Genesis Launch Succeeded! Address: ${deployedAddress}`, "success");

        // 5. POST the generated keys to the local server configuration relative to the address
        log("Saving generated keys locally inside config/user_config.json...");
        const saveRes = await fetch("/api/save-keys", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                address: deployedAddress,
                keys: generatedKeys,
                name: tokenNameStr
            })
        });

        if (saveRes.ok) {
            log(`Successfully saved keys for ${tokenNameStr} (${deployedAddress}) in your private config.`, "success");
            // Reload keys list
            loadConfigKeys();
        } else {
            const errData = await saveRes.json();
            log(`Failed to save keys: ${errData.error}`, "error");
        }

    } catch (err) {
        log(`Genesis Launch failed: ${err.message}`, "error");
    }
});

// Initialize on Load
loadConfigKeys();
