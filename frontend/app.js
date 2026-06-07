const FACTORY_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
let PKMINTER_ADDRESS = "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B";

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

let config = {};

// Load Config Keys from Local Server
async function loadConfigKeys() {
    try {
        const res = await fetch("/api/config");
        if (!res.ok) throw new Error("Failed to fetch config");
        config = await res.json();
        if (config.networks && config.networks.localhost) {
            PKMINTER_ADDRESS = config.networks.localhost.pkminterAddress || PKMINTER_ADDRESS;
        }
        
        keysList.innerHTML = "";
        walletAddresses = [];
        
        // Disregard default static config keys, map dynamically from saved_keys
        const savedKeysMap = config.saved_keys || {};
        const addressList = Object.keys(savedKeysMap);
        
        if (addressList.length === 0) {
            keysList.innerHTML = `<div class="balance-item placeholder">No dynamically deployed metadata key sets found.</div>`;
            return;
        }

        addressList.forEach((address) => {
            const keysMetadata = savedKeysMap[address];
            const keysArray = keysMetadata.privateKeys || keysMetadata;
            
            // Find name alias in localhost config mapping
            let nameAlias = "Genesis PK";
            if (config.networks && config.networks.localhost) {
                for (const [name, val] of Object.entries(config.networks.localhost)) {
                    if (val.toLowerCase() === address.toLowerCase()) {
                        nameAlias = name;
                        break;
                    }
                }
            }

            const sectionDiv = document.createElement("div");
            sectionDiv.style.marginBottom = "20px";
            sectionDiv.style.padding = "10px";
            sectionDiv.style.border = "1px solid rgba(0, 242, 254, 0.15)";
            sectionDiv.style.borderRadius = "8px";
            sectionDiv.style.background = "rgba(0,0,0,0.2)";

            sectionDiv.innerHTML = `
                <div style="font-family: 'Orbitron', sans-serif; font-size: 0.95rem; color: var(--neon-blue); margin-bottom: 8px;">
                    ${nameAlias}
                </div>
                <div style="font-size: 0.8rem; color: var(--text-secondary); margin-bottom: 10px; word-break: break-all;">
                    Address: ${address}
                </div>
                <div class="keys-grid" style="display: flex; flex-direction: column; gap: 8px;"></div>
            `;

            const gridDiv = sectionDiv.querySelector(".keys-grid");

            keysArray.forEach((key, index) => {
                const wallet = new ethers.Wallet(key);
                // Dynamically build walletAddresses list for standard index usage
                walletAddresses.push(wallet.address);
                
                const item = document.createElement("div");
                item.className = "key-item";
                item.style.padding = "8px";
                item.style.background = "rgba(255,255,255,0.02)";
                item.innerHTML = `
                    <div class="key-index" style="font-size: 0.8rem;">Key #${index + 1}</div>
                    <div class="key-address" style="font-size: 0.85rem;">Address: ${wallet.address}</div>
                    <div class="key-privkey" style="font-size: 0.75rem;">Private: ${key.substring(0, 8)}...${key.substring(key.length - 8)}</div>
                `;
                gridDiv.appendChild(item);
            });
            keysList.appendChild(sectionDiv);
        });

        log(`Loaded ${addressList.length} deployed metadata key sets successfully.`, "success");
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
            if (config.networks && config.networks.localhost) {
                PKMINTER_ADDRESS = config.networks.localhost.pkminterAddress || PKMINTER_ADDRESS;
            }
        } else if (network.chainId === 369n) {
            networkName = "PulseChain Mainnet";
            if (config.networks && config.networks.pulsechain) {
                PKMINTER_ADDRESS = config.networks.pulsechain.pkminterAddress || "0x9f4E1471e614747A9a56A33eb0338671ebA1dE2B";
            }
        }
        
        networkBadge.innerText = networkName;
        networkBadge.className = "network-badge connected";
        log(`Active network: ${networkName} (Chain ID: ${network.chainId})`, "info");
    } catch (err) {
        log(`Connection failed: ${err.message}`, "error");
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
    const salt = create2Salt.value.trim();
    let bytecode = create2Bytecode.value.trim() || create2Bytecode.placeholder;

    log("Broadcasting deployment via Yul factory...");
    try {
        let activeSigner = signer;
        if (!activeSigner) {
            log("No browser wallet connected. Falling back to local deployer wallet...", "info");
            const localProvider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            const deployerPrivateKey = "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d";
            activeSigner = new ethers.Wallet(deployerPrivateKey, localProvider);
        }

        const iface = new ethers.Interface([
            "function executeDeployment(bytes bytecode, bytes32 salt) external returns (address)"
        ]);
        const calldata = iface.encodeFunctionData("executeDeployment", [bytecode, salt]);

        log("Sending transaction...");
        const tx = await activeSigner.sendTransaction({
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
                    addresses: activeAddresses,
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
                addresses: generatedAddresses,
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

// GNSS Tracker WebSocket Client Interface
let gpsWs = null;

const wsUrlEl = document.getElementById("wsUrl");
const btnConnectWsEl = document.getElementById("btnConnectWs");
const gpsTimeEl = document.getElementById("gpsTime");
const gpsSatsEl = document.getElementById("gpsSats");
const gpsLatEl = document.getElementById("gpsLat");
const gpsNSEl = document.getElementById("gpsNS");
const gpsLonEl = document.getElementById("gpsLon");
const gpsEWEl = document.getElementById("gpsEW");
const gpsAltEl = document.getElementById("gpsAlt");
const gpsFixEl = document.getElementById("gpsFix");
const wsCmdEl = document.getElementById("wsCmd");
const btnSendCmdEl = document.getElementById("btnSendCmd");
const btnClearGpsLogsEl = document.getElementById("btnClearGpsLogs");
const gpsLogsEl = document.getElementById("gpsLogs");

function logGps(msg, type = "info") {
    const line = document.createElement("div");
    line.className = `log-line ${type}`;
    if (type === "error") line.style.color = "var(--neon-magenta)";
    else if (type === "success") line.style.color = "#00ff7f";
    else if (type === "warning") line.style.color = "var(--neon-purple)";
    
    line.innerText = msg;
    gpsLogsEl.appendChild(line);
    gpsLogsEl.scrollTop = gpsLogsEl.scrollHeight;
}

btnClearGpsLogsEl.addEventListener("click", () => {
    gpsLogsEl.innerHTML = "";
    logGps("Console logs cleared.");
});

btnConnectWsEl.addEventListener("click", () => {
    if (gpsWs && (gpsWs.readyState === WebSocket.OPEN || gpsWs.readyState === WebSocket.CONNECTING)) {
        logGps("Disconnecting from GPS WebSocket...");
        gpsWs.close();
        return;
    }

    const url = wsUrlEl.value.trim();
    logGps(`Connecting to WebSocket: ${url}...`);
    btnConnectWsEl.innerText = "Connecting";
    btnConnectWsEl.style.background = "var(--neon-purple)";

    try {
        gpsWs = new WebSocket(url);
        
        gpsWs.onopen = () => {
            logGps("Connected to board WebSocket!", "success");
            btnConnectWsEl.innerText = "Disconnect";
            btnConnectWsEl.style.background = "var(--neon-magenta)";
            btnConnectWsEl.style.color = "#fff";
        };

        gpsWs.onclose = () => {
            logGps("Disconnected from WebSocket.");
            btnConnectWsEl.innerText = "Connect";
            btnConnectWsEl.style.background = "";
            btnConnectWsEl.style.color = "";
            gpsWs = null;
        };

        gpsWs.onerror = (err) => {
            logGps(`WebSocket error.`, "error");
        };

        gpsWs.onmessage = (event) => {
            const dataStr = event.data.trim();
            
            // Check if JSON telemetry packet
            if (dataStr.startsWith("{") && dataStr.endsWith("}")) {
                try {
                    const parsed = JSON.parse(dataStr);
                    if (parsed.type === "status") {
                        gpsTimeEl.innerText = parsed.time || "N/A";
                        gpsSatsEl.innerText = parsed.sats || "0";
                        gpsLatEl.innerText = parsed.lat || "N/A";
                        gpsNSEl.innerText = parsed.ns || "";
                        gpsLonEl.innerText = parsed.lon || "N/A";
                        gpsEWEl.innerText = parsed.ew || "";
                        gpsAltEl.innerText = parsed.alt || "0.0";
                        
                        const hasFix = parsed.fix !== "0";
                        gpsFixEl.innerText = hasFix ? "Fix OK" : "No Fix";
                        gpsFixEl.style.color = hasFix ? "#00ff7f" : "var(--neon-magenta)";
                        return;
                    } else if (parsed.type === "wifi_scan_results") {
                        const selectEl = document.getElementById("wifiNetworkSelect");
                        const scanBtn = document.getElementById("btnScanWifi");
                        
                        selectEl.innerHTML = '<option value="">Select a Network...</option>';
                        parsed.networks.forEach(net => {
                            const option = document.createElement("option");
                            option.value = net.ssid;
                            option.innerText = `${net.ssid} (${net.rssi} dBm)`;
                            selectEl.appendChild(option);
                        });
                        
                        scanBtn.innerText = "Scan";
                        scanBtn.disabled = false;
                        logGps(`WiFi Scan finished: Found ${parsed.networks.length} networks.`, "success");
                        return;
                    } else if (parsed.type === "wifi_scan_err") {
                        const scanBtn = document.getElementById("btnScanWifi");
                        scanBtn.innerText = "Scan";
                        scanBtn.disabled = false;
                        logGps(`WiFi Scan error: ${parsed.msg}`, "error");
                        return;
                    } else if (parsed.type === "wifi_connected") {
                        const statusEl = document.getElementById("wifiStatus");
                        statusEl.innerText = `Board Status: Connected (IP: ${parsed.ip})`;
                        statusEl.style.color = "#00ff7f";
                        logGps(`Board connected to LAN. IP Address: ${parsed.ip}`, "success");
                        return;
                    }
                } catch (e) {
                    // Ignore JSON parse errors, treat as raw message
                }
            }
            
            // Print to GPS stream log
            logGps(dataStr);
        };
    } catch (err) {
        logGps(`Failed to create WebSocket connection: ${err.message}`, "error");
        btnConnectWsEl.innerText = "Connect";
        btnConnectWsEl.style.background = "";
    }
});

btnSendCmdEl.addEventListener("click", () => {
    if (!gpsWs || gpsWs.readyState !== WebSocket.OPEN) {
        logGps("Error: WebSocket is not connected.", "error");
        return;
    }

    const cmd = wsCmdEl.value.trim();
    if (!cmd) return;

    gpsWs.send(cmd);
    wsCmdEl.value = "";
});

wsCmdEl.addEventListener("keypress", (e) => {
    if (e.key === "Enter") {
        btnSendCmdEl.click();
    }
});

// WiFi Scanner & Connector Button Event Listeners
const btnScanWifiEl = document.getElementById("btnScanWifi");
const btnConnectWifiEl = document.getElementById("btnConnectWifi");
const wifiNetworkSelectEl = document.getElementById("wifiNetworkSelect");
const wifiPassEl = document.getElementById("wifiPass");
const wifiStatusEl = document.getElementById("wifiStatus");

btnScanWifiEl.addEventListener("click", () => {
    if (!gpsWs || gpsWs.readyState !== WebSocket.OPEN) {
        logGps("Error: WebSocket is not connected. Connect to the board AP first.", "error");
        return;
    }
    
    logGps("Requesting board to scan Wi-Fi networks...");
    btnScanWifiEl.innerText = "Scanning...";
    btnScanWifiEl.disabled = true;
    gpsWs.send(":wifi_scan");
});

btnConnectWifiEl.addEventListener("click", () => {
    if (!gpsWs || gpsWs.readyState !== WebSocket.OPEN) {
        logGps("Error: WebSocket is not connected.", "error");
        return;
    }
    
    const selectedSsid = wifiNetworkSelectEl.value;
    const password = wifiPassEl.value;
    
    if (!selectedSsid) {
        logGps("Error: Please select a Wi-Fi network from the dropdown.", "error");
        return;
    }
    
    logGps(`Connecting board to Wi-Fi SSID: '${selectedSsid}'...`);
    wifiStatusEl.innerText = "Board Status: Connecting...";
    wifiStatusEl.style.color = "var(--neon-purple)";
    
    // Command structure: :wifi_connect <ssid> <password>
    gpsWs.send(`:wifi_connect ${selectedSsid} ${password}`);
    wifiPassEl.value = "";
});

// ============================================================================
// DATAMOST SIMULATOR EMBEDDED ENGINE
// ============================================================================
let datamostProvider, datamostSigner;
let datamostConfig = {};
let audioCtx;
let isAudioPlaying = false;
let voices = [];

class SIDVoice {
    constructor(audioCtx, destination) {
        this.ctx = audioCtx;
        this.dest = destination;
        this.osc = null;
        this.gain = null;
        this.gate = 0;
        this.lastFreq = 0;
        this.lastWave = 0;
    }

    update(freq, control, ad, sr) {
        if (!this.ctx) return;
        const gate = control & 1;
        const waveBits = control & 0xF0;
        
        let type = 'sawtooth';
        if (waveBits & 0x10) type = 'triangle';
        else if (waveBits & 0x20) type = 'sawtooth';
        else if (waveBits & 0x40) type = 'square';
        else if (waveBits & 0x80) type = 'triangle'; 

        const freqHz = Math.round(freq * 0.06097);

        if (freqHz <= 0 || waveBits === 0) {
            if (this.osc) {
                this.osc.stop();
                this.osc.disconnect();
                this.osc = null;
            }
            return;
        }

        if (!this.osc || this.lastWave !== waveBits) {
            if (this.osc) {
                this.osc.stop();
                this.osc.disconnect();
            }
            this.osc = this.ctx.createOscillator();
            this.gain = this.ctx.createGain();
            this.osc.type = type;
            this.osc.connect(this.gain);
            this.gain.connect(this.dest);
            this.gain.gain.setValueAtTime(0, this.ctx.currentTime);
            this.osc.start();
            this.lastWave = waveBits;
            this.gate = 0;
        }

        if (freqHz !== this.lastFreq) {
            this.osc.frequency.setValueAtTime(freqHz, this.ctx.currentTime);
            this.lastFreq = freqHz;
        }

        if (gate !== this.gate) {
            const now = this.ctx.currentTime;
            const attackIdx = (ad >> 4) & 0xF;
            const decayIdx = ad & 0xF;
            const sustainIdx = (sr >> 4) & 0xF;
            const releaseIdx = sr & 0xF;

            const attackDurations = [0.002, 0.008, 0.016, 0.024, 0.038, 0.056, 0.068, 0.08, 0.1, 0.25, 0.5, 0.8, 1.0, 3.0, 5.0, 8.0];
            const decayDurations = [0.006, 0.024, 0.048, 0.072, 0.114, 0.168, 0.204, 0.24, 0.3, 0.75, 1.5, 2.4, 3.0, 9.0, 15.0, 24.0];
            const sustainLevels = [0, 0.06, 0.13, 0.2, 0.26, 0.33, 0.4, 0.46, 0.53, 0.6, 0.66, 0.73, 0.8, 0.86, 0.93, 1.0];

            const aTime = attackDurations[attackIdx];
            const dTime = decayDurations[decayIdx];
            const sVol = sustainLevels[sustainIdx] * 0.15;
            const rTime = decayDurations[releaseIdx];

            this.gain.gain.cancelScheduledValues(now);

            if (gate === 1) {
                this.gain.gain.setValueAtTime(this.gain.gain.value, now);
                this.gain.gain.linearRampToValueAtTime(0.15, now + aTime);
                this.gain.gain.exponentialRampToValueAtTime(sVol + 0.001, now + aTime + dTime);
            } else {
                this.gain.gain.setValueAtTime(this.gain.gain.value, now);
                this.gain.gain.exponentialRampToValueAtTime(0.0001, now + rTime);
            }
            this.gate = gate;
        }
    }

    stop() {
        if (this.osc) {
            this.osc.stop();
            this.osc.disconnect();
            this.osc = null;
        }
    }
}

// ABIs for Yul Contracts
const cpuABI = [
    "function executeOp(uint8 opcode, uint256 operand) public returns (uint256)",
    "function getCPUState() public view returns (uint256, uint256, uint256, uint256, uint256, uint256)",
    "function runSteps(uint256 maxSteps) public returns (uint256)",
    "function poke(uint256 addr, uint256 val) public returns (uint256)",
    "function getScreenRAM() public view returns (bytes)",
    "function getColorRAM() public view returns (bytes)"
];
const graphicsABI = [
    "function updateSprite(uint8 index, uint16 x, uint8 y) public returns (uint256)",
    "function checkCollisions() public returns (uint256)",
    "function setSpriteRow(uint8 index, uint8 row, uint256 val) public returns (uint256)",
    "function setSpritePatternFull(uint8 index, uint256[21] rows) public returns (uint256)"
];
const musicABI = [
    "function getVoice1Frequency() public view returns (uint256)",
    "function poke(uint16 addr, uint8 val) public returns (uint256)",
    "function getSIDState() public view returns (uint256[25])"
];

let cpuContract, graphicsContract, musicContract;
let spritePatterns = [[], [], [], [], [], [], [], []];

async function initDatamostConsole() {
    log("Initializing DATAMOST simulator console on main dashboard...", "info");
    drawVulkanPlaceholder();
    
    const hostname = "127.0.0.1";
    datamostProvider = new ethers.JsonRpcProvider("http://" + hostname + ":8545");
    
    const fallbackPrivateKey = "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d";
    datamostSigner = new ethers.Wallet(fallbackPrivateKey, datamostProvider);

    try {
        const res = await fetch("/api/config");
        datamostConfig = await res.json();
        const localhost = datamostConfig.networks.localhost;

        if (localhost.cpu6502Address && localhost.graphicsSystemAddress && localhost.musicMakerAddress) {
            cpuContract = new ethers.Contract(localhost.cpu6502Address, cpuABI, datamostSigner);
            graphicsContract = new ethers.Contract(localhost.graphicsSystemAddress, graphicsABI, datamostSigner);
            musicContract = new ethers.Contract(localhost.musicMakerAddress, musicABI, datamostSigner);
            log("DATAMOST system drivers successfully loaded.", "success");

            // Upload 6502 program to memory
            const invadersProgram = [
                0xAD, 0x03, 0xD0, // LDA $D003
                0xC9, 0xC8,       // CMP #200
                0xB0, 0x0A,       // BCS ALIEN
                0xAD, 0x03, 0xD0, // LDA $D003
                0x38,             // SEC
                0xE9, 0x06,       // SBC #6
                0x8D, 0x03, 0xD0, // STA $D003
                
                0xA5, 0x02,       // LDA $02 (direction)
                0xD0, 0x14,       // BNE RIGHT
                // LEFT
                0xAD, 0x04, 0xD0, // LDA $D004
                0x38,             // SEC
                0xE9, 0x03,       // SBC #3
                0x8D, 0x04, 0xD0, // STA $D004
                0xC9, 0x28,       // CMP #40
                0xB0, 0x18,       // BCS COLL
                0xA9, 0x01,       // LDA #1
                0x85, 0x02,       // STA $02
                0x4C, 0x3A, 0x20, // JMP COLL
                // RIGHT
                0xAD, 0x04, 0xD0, // LDA $D004
                0x18,             // CLC
                0x69, 0x03,       // ADC #3
                0x8D, 0x04, 0xD0, // STA $D004
                0xC9, 0xF0,       // CMP #240
                0x90, 0x04,       // BCC COLL
                0xA9, 0x00,       // LDA #0
                0x85, 0x02,       // STA $02

                // COLL
                0xAD, 0x1E, 0xD0, // LDA $D01E
                0x29, 0x06,       // AND #6
                0xC9, 0x06,       // CMP #6
                0xD0, 0x0D,       // BNE PLAYER
                0xA9, 0xC8,       // LDA #200
                0x8D, 0x03, 0xD0, // STA $D003
                0xA9, 0x3C,       // LDA #60
                0x8D, 0x04, 0xD0, // STA $D004
                0xEE, 0x03, 0x00, // INC $0003

                // PLAYER
                0xAD, 0x00, 0xDC, // LDA $DC00
                0x29, 0x04,       // AND #4 (Left)
                0xD0, 0x09,       // BNE CHECK_RIGHT
                0xAD, 0x00, 0xD0, // LDA $D000
                0x38,             // SEC
                0xE9, 0x05,       // SBC #5
                0x8D, 0x00, 0xD0, // STA $D000
                // CHECK_RIGHT
                0xAD, 0x00, 0xDC, // LDA $DC00
                0x29, 0x08,       // AND #8 (Right)
                0xD0, 0x09,       // BNE CHECK_FIRE
                0xAD, 0x00, 0xD0, // LDA $D000
                0x18,             // CLC
                0x69, 0x05,       // ADC #5
                0x8D, 0x00, 0xD0, // STA $D000
                // CHECK_FIRE
                0xAD, 0x00, 0xDC, // LDA $DC00
                0x29, 0x10,       // AND #16 (Fire)
                0xD0, 0x0B,       // BNE END
                0xAD, 0x00, 0xD0, // LDA $D000
                0x8D, 0x02, 0xD0, // STA $D002
                0xA9, 0x87,       // LDA #135
                0x8D, 0x03, 0xD0, // STA $D003
                0x00              // BRK
            ];

            for (let i = 0; i < invadersProgram.length; i++) {
                await cpuContract.poke(8192 + i, invadersProgram[i]);
            }
            await cpuContract.poke(53248, 120); // Player X
            await cpuContract.poke(53252, 100); // Alien X
            await cpuContract.poke(56320, 0xFF); // Joystick inputs
            await cpuContract.poke(0x02, 0);    // Alien direction = 0

            // Pre-fetch Sprite Patterns
            for (let i = 0; i < 8; i++) {
                spritePatterns[i] = [];
                for (let r = 0; r < 21; r++) {
                    try {
                        const val = Number(await datamostProvider.getStorage(graphicsContract.target, "0x" + (54000 + i * 32 + r).toString(16)));
                        spritePatterns[i].push(val);
                    } catch (err) {
                        spritePatterns[i].push(0);
                    }
                }
            }

            startDatamostSimulationLoops();
        } else {
            log("Yul contract addresses not found in registry. Please run Yul deployment script.", "error");
        }
    } catch (err) {
        log(`Failed to initialize Datamost console: ${err.message}`, "error");
    }
}

// Audio Toggle Event Listener
document.getElementById("btnAudioToggle").addEventListener("click", () => {
    if (!isAudioPlaying) {
        if (!audioCtx) {
            audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        audioCtx.resume();
        voices = [
            new SIDVoice(audioCtx, audioCtx.destination),
            new SIDVoice(audioCtx, audioCtx.destination),
            new SIDVoice(audioCtx, audioCtx.destination)
        ];
        
        isAudioPlaying = true;
        document.getElementById("btnAudioToggle").innerText = "Mute Audio";
        document.getElementById("btnAudioToggle").classList.add("btn-danger");
        log("Web Audio SID emulator started.", "info");
    } else {
        voices.forEach(v => v.stop());
        voices = [];
        isAudioPlaying = false;
        document.getElementById("btnAudioToggle").innerText = "Enable Audio Synth";
        document.getElementById("btnAudioToggle").classList.remove("btn-danger");
        log("Web Audio SID emulator muted.", "info");
    }
});

// Run 6502 Opcode Event Listener
document.getElementById("btnExecuteOp").addEventListener("click", async () => {
    if (!cpuContract) return;
    const opcode = parseInt(document.getElementById("selectOpcode").value, 16);
    const operand = parseInt(document.getElementById("inputOperand").value);

    try {
        log(`Executing 6502 opcode ${opcode.toString(16).toUpperCase()} with operand ${operand}...`, "info");
        let activeSigner = signer;
        if (!activeSigner) {
            const localProvider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            activeSigner = new ethers.Wallet("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d", localProvider);
        }
        const userCpuContract = new ethers.Contract(cpuContract.target, cpuABI, activeSigner);
        const tx = await userCpuContract.executeOp(opcode, operand);
        await tx.wait();
        log("Opcode successfully executed on-chain.", "success");
    } catch (err) {
        log(`Opcode execution failed: ${err.message}`, "error");
    }
});

// Move Sprite Event Listener
document.getElementById("btnMoveSprite").addEventListener("click", async () => {
    if (!graphicsContract) return;
    const spriteId = parseInt(document.getElementById("selectSprite").value);
    const x = parseInt(document.getElementById("spriteX").value);
    const y = parseInt(document.getElementById("spriteY").value);

    try {
        log(`Moving Sprite ${spriteId} to coordinates (${x}, ${y})...`, "info");
        let activeSigner = signer;
        if (!activeSigner) {
            const localProvider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            activeSigner = new ethers.Wallet("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d", localProvider);
        }
        const userGraphicsContract = new ethers.Contract(graphicsContract.target, graphicsABI, activeSigner);
        const tx = await userGraphicsContract.updateSprite(spriteId, x, y);
        await tx.wait();
        log(`Sprite ${spriteId} successfully updated on-chain.`, "success");
    } catch (err) {
        log(`Sprite update failed: ${err.message}`, "error");
    }
});

// Game coordinates and keyboard listener
let s0_x = 120;
let s0_y = 150;
let s1_x = 120;
let s1_y = 200;
let s2_x = 60;
let s2_y = 60;

window.addEventListener("keydown", async (e) => {
    if (!cpuContract || !graphicsContract || !musicContract) return;

    if (document.activeElement && (document.activeElement.tagName === "INPUT" || document.activeElement.tagName === "SELECT" || document.activeElement.tagName === "TEXTAREA")) {
        return;
    }

    let joystick = 0xFF; 
    let moved = false;

    if (e.key === "ArrowLeft" || e.key === "a") {
        joystick = joystick & ~0x04; 
        playSynthTone(180, 150);
        pokeSIDFreq(1800, 0x21);
        moved = true;
    } else if (e.key === "ArrowRight" || e.key === "d") {
        joystick = joystick & ~0x08; 
        playSynthTone(180, 150);
        pokeSIDFreq(1800, 0x21);
        moved = true;
    } else if (e.key === " " || e.key === "Spacebar") {
        e.preventDefault();
        joystick = joystick & ~0x10; 
        playLaserSweep();
        pokeSIDFreq(6000, 0x11);
        moved = true;
    }

    if (moved) {
        try {
            await cpuContract.poke(56320, joystick);
        } catch (err) {
            console.error("Joystick poke failed:", err);
        }
    }
});

window.addEventListener("keyup", async (e) => {
    if (!cpuContract) return;
    if (e.key === "ArrowLeft" || e.key === "a" || e.key === "ArrowRight" || e.key === "d" || e.key === " " || e.key === "Spacebar") {
        try {
            await cpuContract.poke(56320, 0xFF);
        } catch (err) {}
    }
});

function playSynthTone(freqHz, durationMs) {
    if (isAudioPlaying && audioCtx) {
        try {
            let osc = audioCtx.createOscillator();
            let gainNode = audioCtx.createGain();
            osc.type = "sawtooth";
            osc.frequency.setValueAtTime(freqHz, audioCtx.currentTime);
            gainNode.gain.setValueAtTime(0.15, audioCtx.currentTime);
            gainNode.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + durationMs/1000);
            osc.connect(gainNode);
            gainNode.connect(audioCtx.destination);
            osc.start();
            osc.stop(audioCtx.currentTime + durationMs/1000);
        } catch (e) {}
    }
}

function playLaserSweep() {
    if (isAudioPlaying && audioCtx) {
        try {
            let osc = audioCtx.createOscillator();
            let gainNode = audioCtx.createGain();
            osc.type = "triangle";
            osc.frequency.setValueAtTime(1200, audioCtx.currentTime);
            osc.frequency.exponentialRampToValueAtTime(150, audioCtx.currentTime + 0.3);
            gainNode.gain.setValueAtTime(0.2, audioCtx.currentTime);
            gainNode.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + 0.3);
            osc.connect(gainNode);
            gainNode.connect(audioCtx.destination);
            osc.start();
            osc.stop(audioCtx.currentTime + 0.3);
        } catch (e) {}
    }
}

async function pokeSIDFreq(freqVal, controlVal = 0x21) {
    try {
        const lo = freqVal & 0xFF;
        const hi = (freqVal >> 8) & 0xFF;
        await musicContract.poke(54272, lo);
        await musicContract.poke(54273, hi);
        await musicContract.poke(54276, controlVal);
    } catch (err) {
        console.error("SID poke failed:", err);
    }
}

const c64Canvas = document.getElementById("c64Screen");
const c64Ctx = c64Canvas.getContext("2d");

const c64Palette = [
    "#000000", // 0: Black
    "#FFFFFF", // 1: White
    "#880000", // 2: Red
    "#AAFFEE", // 3: Cyan
    "#CC44CC", // 4: Purple
    "#00CC55", // 5: Green
    "#0000AA", // 6: Blue
    "#EEEE77", // 7: Yellow
    "#DD8855", // 8: Orange
    "#664400", // 9: Brown
    "#FF7777", // 10: Light Red
    "#333333", // 11: Dark Grey
    "#777777", // 12: Grey
    "#AAFF66", // 13: Light Green
    "#0088FF", // 14: Light Blue
    "#BBBBBB"  // 15: Light Grey
];

function startDatamostSimulationLoops() {
    setInterval(async () => {
        if (!datamostProvider) return;

        // 1. Poll CPU registers
        if (cpuContract) {
            try {
                const state = await cpuContract.getCPUState();
                document.getElementById("regA").innerText = Number(state[0]).toString(16).toUpperCase().padStart(2, '0');
                document.getElementById("regX").innerText = Number(state[1]).toString(16).toUpperCase().padStart(2, '0');
                document.getElementById("regY").innerText = Number(state[2]).toString(16).toUpperCase().padStart(2, '0');
                document.getElementById("regSR").innerText = Number(state[3]).toString(16).toUpperCase().padStart(2, '0');
                document.getElementById("regSP").innerText = Number(state[4]).toString(16).toUpperCase().padStart(2, '0');
                document.getElementById("regPC").innerText = Number(state[5]).toString(16).toUpperCase().padStart(4, '0');
            } catch (err) {}
        }

        // 2. Poll audio registers
        if (musicContract) {
            try {
                const state = await musicContract.getSIDState();
                
                const v1_freq = (Number(state[1]) << 8) | Number(state[0]);
                const v1_ctrl = Number(state[4]);
                const v1_ad = Number(state[5]);
                const v1_sr = Number(state[6]);
                
                const v2_freq = (Number(state[8]) << 8) | Number(state[7]);
                const v2_ctrl = Number(state[11]);
                const v2_ad = Number(state[12]);
                const v2_sr = Number(state[13]);
                
                const v3_freq = (Number(state[15]) << 8) | Number(state[14]);
                const v3_ctrl = Number(state[18]);
                const v3_ad = Number(state[19]);
                const v3_sr = Number(state[20]);

                const freqHz = Math.round(v1_freq * 0.06097);
                document.getElementById("sidFreqText").innerText = freqHz;

                if (isAudioPlaying && voices.length === 3) {
                    voices[0].update(v1_freq, v1_ctrl, v1_ad, v1_sr);
                    voices[1].update(v2_freq, v2_ctrl, v2_ad, v2_sr);
                    voices[2].update(v3_freq, v3_ctrl, v3_ad, v3_sr);
                }
            } catch (err) {}
        }

        // 3. Physics update
        if (graphicsContract && cpuContract) {
            try {
                const collisionMask = Number(await graphicsContract.checkCollisions.staticCall());
                const isCollision = (collisionMask & 6) === 6;

                await cpuContract.poke(53278, collisionMask);
                const tx = await cpuContract.runSteps(120);
                await tx.wait();

                const spr0_x = Number(await datamostProvider.getStorage(cpuContract.target, "0xd000"));
                const spr0_y = Number(await datamostProvider.getStorage(cpuContract.target, "0xd001"));
                const spr1_x = Number(await datamostProvider.getStorage(cpuContract.target, "0xd002"));
                const spr1_y = Number(await datamostProvider.getStorage(cpuContract.target, "0xd003"));
                const spr2_x = Number(await datamostProvider.getStorage(cpuContract.target, "0xd004"));
                const spr2_y = Number(await datamostProvider.getStorage(cpuContract.target, "0xd005"));

                s0_x = spr0_x;
                s0_y = spr0_y;
                s1_x = spr1_x;
                s1_y = spr1_y;
                s2_x = spr2_x;
                s2_y = spr2_y;

                await graphicsContract.updateSprite(0, s0_x, s0_y);
                await graphicsContract.updateSprite(1, s1_x, s1_y);
                await graphicsContract.updateSprite(2, s2_x, s2_y);

                // Fetch border and background color registers (53280 and 53281)
                const borderVal = Number(await datamostProvider.getStorage(cpuContract.target, "0xd020")) & 0xF;
                const bgVal = Number(await datamostProvider.getStorage(cpuContract.target, "0xd021")) & 0xF;
                const borderHex = c64Palette[borderVal] || c64Palette[14];
                const bgHex = c64Palette[bgVal] || c64Palette[6];

                // Fetch Screen RAM and Color RAM
                let screenRam = new Uint8Array(1000);
                let colorRam = new Uint8Array(1000);
                try {
                    const screenHex = await cpuContract.getScreenRAM();
                    const colorHex = await cpuContract.getColorRAM();
                    screenRam = ethers.getBytes(screenHex);
                    colorRam = ethers.getBytes(colorHex);
                } catch (e) {}

                const score = Number(await datamostProvider.getStorage(cpuContract.target, "0x0003"));
                const colStatusText = document.getElementById("collisionStatus");
                colStatusText.innerText = `SCORE: ${score} | Collision Mask: ${collisionMask}`;
                if (isCollision) {
                    colStatusText.style.color = "var(--neon-magenta)";
                    playSynthTone(100, 500); 
                    pokeSIDFreq(1000, 0x21);
                } else {
                    colStatusText.style.color = "var(--neon-blue)";
                }

                drawC64Canvas(s0_x, s0_y, s1_x, s1_y, s2_x, s2_y, isCollision, borderHex, bgHex, screenRam, colorRam);
            } catch (err) {}
        }
        updateVulkanScreen();
    }, 400);
}

const vulkanCanvas = document.getElementById("vulkanScreen");
const vulkanCtx = vulkanCanvas ? vulkanCanvas.getContext("2d") : null;

function drawVulkanPlaceholder() {
    if (!vulkanCanvas || !vulkanCtx) return;
    vulkanCtx.fillStyle = "#110b29"; 
    vulkanCtx.fillRect(0, 0, vulkanCanvas.width, vulkanCanvas.height);
    vulkanCtx.strokeStyle = "rgba(0, 242, 254, 0.15)";
    vulkanCtx.lineWidth = 1;
    for (let i = 0; i < vulkanCanvas.width; i += 20) {
        vulkanCtx.beginPath();
        vulkanCtx.moveTo(i, 0);
        vulkanCtx.lineTo(i, vulkanCanvas.height);
        vulkanCtx.stroke();
    }
    for (let j = 0; j < vulkanCanvas.height; j += 20) {
        vulkanCtx.beginPath();
        vulkanCtx.moveTo(0, j);
        vulkanCtx.lineTo(vulkanCanvas.width, j);
        vulkanCtx.stroke();
    }
    vulkanCtx.fillStyle = "#00f2fe";
    vulkanCtx.font = "bold 11px 'Orbitron', monospace";
    vulkanCtx.textAlign = "center";
    vulkanCtx.fillText("VULKAN COMPATIBLE DATAMOST DISPLAY", vulkanCanvas.width / 2, vulkanCanvas.height / 2 - 10);
    vulkanCtx.fillStyle = "#ff007f";
    vulkanCtx.fillText("LOADING STREAM FROM HOST...", vulkanCanvas.width / 2, vulkanCanvas.height / 2 + 10);
}

async function updateVulkanScreen() {
    try {
        const res = await fetch("/vulkan_frame.ppm");
        if (!res.ok) return;
        const buffer = await res.arrayBuffer();
        const arr = new Uint8Array(buffer);
        
        let pos = 0;
        function nextToken() {
            while (pos < arr.length && arr[pos] <= 32) { pos++; } 
            let start = pos;
            while (pos < arr.length && arr[pos] > 32) { pos++; }
            let bytes = arr.subarray(start, pos);
            let str = "";
            for (let i = 0; i < bytes.length; i++) {
                str += String.fromCharCode(bytes[i]);
            }
            return str;
        }

        const magic = nextToken();
        if (magic !== "P6") return;
        const width = parseInt(nextToken(), 10);
        const height = parseInt(nextToken(), 10);
        const maxVal = nextToken(); 
        pos++; 
        
        const pixelData = arr.subarray(pos);
        if (pixelData.length < width * height * 3) return;

        const imgData = vulkanCtx.createImageData(width, height);
        const data = imgData.data;
        let srcIdx = 0;
        for (let i = 0; i < width * height * 4; i += 4) {
            data[i] = pixelData[srcIdx];     
            data[i+1] = pixelData[srcIdx+1]; 
            data[i+2] = pixelData[srcIdx+2]; 
            data[i+3] = 255;                 
            srcIdx += 3;
        }
        
        const tempCanvas = document.createElement("canvas");
        tempCanvas.width = width;
        tempCanvas.height = height;
        const tempCtx = tempCanvas.getContext("2d");
        tempCtx.putImageData(imgData, 0, 0);
        
        vulkanCtx.drawImage(tempCanvas, 0, 0, vulkanCanvas.width, vulkanCanvas.height);
    } catch (err) {}
}

function drawC64Canvas(s0_x, s0_y, s1_x, s1_y, s2_x, s2_y, hasCollision, borderCol, bgCol, screenRam, colorRam) {
    c64Ctx.fillStyle = borderCol || "#3b5dc9"; 
    c64Ctx.fillRect(0, 0, c64Canvas.width, c64Canvas.height);

    c64Ctx.fillStyle = bgCol || "#a2b4c2"; 
    c64Ctx.fillRect(40, 30, 240, 140);

    // Draw characters from Screen RAM (40 columns by 25 rows)
    if (screenRam && colorRam) {
        c64Ctx.font = "bold 5.5px monospace";
        c64Ctx.textAlign = "left";
        c64Ctx.textBaseline = "top";
        const cellW = 240 / 40; 
        const cellH = 140 / 25; 
        for (let r = 0; r < 25; r++) {
            for (let c = 0; c < 40; c++) {
                const idx = r * 40 + c;
                const charCode = screenRam[idx];
                const colorIdx = colorRam[idx] & 0xF;
                if (charCode > 0) {
                    c64Ctx.fillStyle = c64Palette[colorIdx];
                    let charStr = String.fromCharCode(charCode);
                    c64Ctx.fillText(charStr, 40 + c * cellW, 30 + r * cellH);
                }
            }
        }
    }

    drawC64Sprite(0, s0_x, s0_y, "#ff007f", hasCollision);
    drawC64Sprite(1, s1_x, s1_y, "#00f2fe", hasCollision);
    drawC64Sprite(2, s2_x, s2_y, "#00ff7f", hasCollision);
}

function drawC64Sprite(index, x, y, color, hasCollision) {
    const mappedX = (x % 320);
    const mappedY = (y % 200);

    const pattern = spritePatterns[index] || [];
    let hasPattern = false;
    for (let r = 0; r < pattern.length; r++) {
        if (pattern[r] > 0) {
            hasPattern = true;
            break;
        }
    }

    if (hasPattern) {
        c64Ctx.fillStyle = color;
        for (let r = 0; r < 21; r++) {
            const row_val = pattern[r] || 0;
            for (let c = 0; c < 24; c++) {
                if ((row_val & (1 << (23 - c))) !== 0) {
                    c64Ctx.fillRect(mappedX + c, mappedY + r, 1, 1);
                }
            }
        }
        c64Ctx.strokeStyle = hasCollision ? "#ff007f" : "rgba(255,255,255,0.4)";
        c64Ctx.lineWidth = 1;
        c64Ctx.strokeRect(mappedX - 2, mappedY - 2, 28, 25);
    } else {
        c64Ctx.fillStyle = color;
        c64Ctx.fillRect(mappedX, mappedY, 12, 12);
        c64Ctx.strokeStyle = hasCollision ? "#ff007f" : "rgba(255,255,255,0.4)";
        c64Ctx.lineWidth = 1;
        c64Ctx.strokeRect(mappedX - 2, mappedY - 2, 16, 16);
    }
}

// Initialize on Load
loadConfigKeys();
initDatamostConsole();
