const puppeteer = require('puppeteer-core');
const { spawn } = require('child_process');
const { PNG } = require('pngjs');
const path = require('path');
process.env.TMPDIR = path.join(__dirname, "../tmp");
const readline = require('readline');
const { ethers } = require('ethers');
const http = require('http');

const keepAliveAgent = new http.Agent({ keepAlive: true, maxSockets: 1 });
let hasMjpegClients = false;

let wmqContract = null;
let signer = null;
let provider = null;
let wmqAddressStr = "Not Deployed";
let nextNonce = null;
let wmqEventCount = 0;
let lastBlockNumber = 0;
let presenterWidth = 1024;
let presenterHeight = 768;

async function sendWmqEvent(cmd, args) {
    let fullCmd = `${cmd} ${args}`.trim();
    const isTargetYouTube = fullCmd.startsWith("YOUTUBE:") || fullCmd.startsWith("Y:");
    const port = isTargetYouTube ? 18081 : 18080;
    
    // Direct TCP routing first for zero-latency
    let directRouted = false;
    try {
        const net = require('net');
        await new Promise((resolve, reject) => {
            const socket = net.createConnection({ port, host: '127.0.0.1' }, () => {
                socket.write(fullCmd + '\n');
                socket.end();
                directRouted = true;
                resolve();
            });
            socket.on('error', (err) => {
                reject(err);
            });
            socket.setTimeout(50);
            socket.on('timeout', () => {
                socket.destroy();
                reject(new Error("Timeout"));
            });
        });
    } catch (directErr) {
        // Direct routing failed (e.g. YouTube sub-controller not started yet)
    }

    if (directRouted) {
        return;
    }

    const isIoEvent = cmd.includes("MOUSE") || cmd.includes("KEY") || cmd.includes("MM") || cmd.includes("MD") || cmd.includes("MU") || cmd.includes("MS") || cmd.includes("KD") || cmd.includes("KU");
    if (isIoEvent) {
        return;
    }

    if (!wmqContract || !signer) return;
    try {
        // Abbreviate command prefix and name to fit 32-byte WinchesterMQ limit
        fullCmd = fullCmd
            .replace("YOUTUBE:", "Y:")
            .replace("MAIN:", "M:")
            .replace("MOUSE_MOVE", "MM")
            .replace("MOUSE_DOWN", "MD")
            .replace("MOUSE_UP", "MU")
            .replace("MOUSE_SCROLL", "MS")
            .replace("KEY_DOWN", "KD")
            .replace("KEY_UP", "KU");

        let cmdBytes = ethers.toUtf8Bytes(fullCmd);
        if (cmdBytes.length < 32) {
            const padded = new Uint8Array(32);
            padded.set(cmdBytes);
            cmdBytes = padded;
        } else if (cmdBytes.length > 32) {
            console.warn(`[WMQ WARNING] Command truncated to 32 bytes: ${fullCmd}`);
            cmdBytes = cmdBytes.slice(0, 32);
        }
        const txData = ethers.concat([
            "0xccb077a0", // postEvent
            cmdBytes
        ]);
        const wmqAddress = await wmqContract.getAddress();
        let nonce;
        if (nextNonce !== null) {
            nonce = nextNonce++;
        } else {
            nonce = await provider.getTransactionCount(signer.address, "pending");
        }
        await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 100000, nonce });
    } catch (e) {
        console.error(`[WMQ ERR] Failed to send WinchesterMQ event (${cmd} ${args}): ${e.message}`);
    }
}

const linuxKeyMap = {
    1: 'Escape',
    2: '1', 3: '2', 4: '3', 5: '4', 6: '5', 7: '6', 8: '7', 9: '8', 10: '9', 11: '0',
    12: '-', 13: '=', 14: 'Backspace',
    15: 'Tab',
    16: 'q', 17: 'w', 18: 'e', 19: 'r', 20: 't', 21: 'y', 22: 'u', 23: 'i', 24: 'o', 25: 'p',
    26: '[', 27: ']', 28: 'Enter',
    29: 'Control',
    30: 'a', 31: 's', 32: 'd', 33: 'f', 34: 'g', 35: 'h', 36: 'j', 37: 'k', 38: 'l',
    39: ';', 40: "'", 41: '`',
    42: 'Shift', 43: '\\',
    44: 'z', 45: 'x', 46: 'c', 47: 'v', 48: 'b', 49: 'n', 50: 'm',
    51: ',', 52: '.', 53: '/',
    54: 'Shift', 56: 'Alt', 57: ' ',
    103: 'ArrowUp', 105: 'ArrowLeft', 106: 'ArrowRight', 108: 'ArrowDown'
};

const linuxButtonMap = {
    272: 'left',
    273: 'right',
    274: 'middle'
};

function compileYul(yulPath) {
    const absolutePath = path.resolve(__dirname, yulPath);
    const { execSync } = require('child_process');
    const start = performance.now();
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const elapsed = performance.now() - start;
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    console.log(`[DIAGNOSTIC] Compiled Yul contract ${path.basename(yulPath)} (took ${elapsed.toFixed(3)} ms)`);
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    let url = process.argv[2] || "http://127.0.0.1:8000/zmachine.html";
    const isYouTube = url.includes("youtube.com") || url.includes("youtube");
    const inputQueue = [];
    let processingQueue = false;
    
    // Single-instance lock to maintain Auncient controller integrity and prevent duplicate chromium instances
    const net = require('net');
    const SINGLE_INSTANCE_PORT = isYouTube ? 18081 : 18080;
    
    const isAnotherInstanceRunning = await new Promise((resolve) => {
        const server = net.createServer((socket) => {
            const rl = readline.createInterface({
                input: socket,
                terminal: false
            });
            rl.on('line', (line) => {
                if (line.trim() === 'PING') {
                    handleInputCommand(line, () => {
                        socket.write('PONG\n');
                    });
                } else {
                    handleInputCommand(line);
                }
            });
        });
        server.on('error', (err) => {
            if (err.code === 'EADDRINUSE') {
                resolve(true);
            } else {
                resolve(false);
            }
        });
        server.listen(SINGLE_INSTANCE_PORT, '127.0.0.1', () => {
            resolve(false);
        });
    });

    if (isAnotherInstanceRunning) {
        console.error("[ERROR] Another instance of rooted_browser_controller.js is already running.");
        console.error("Please terminate the existing instance before launching a new one.");
        process.exit(1);
    }

    // Verify local dashboard server is running and spawn if needed
    if (true) {
        const http = require('http');
        const checkServer = () => new Promise((resolve) => {
            const req = http.get("http://127.0.0.1:8000/atropa_dashboard.html", (res) => {
                resolve(res.statusCode === 200);
            });
            req.on('error', () => resolve(false));
            req.setTimeout(1000, () => {
                req.destroy();
                resolve(false);
            });
        });

        let isRunning = await checkServer();
        if (!isRunning) {
            console.log("[SERVER] Local dashboard server is offline. Spawning python3 server.py automatically...");
            const serverProcess = spawn("python3", ["server.py"], {
                cwd: path.join(__dirname, ".."),
                detached: true,
                stdio: 'ignore'
            });
            serverProcess.unref();
            await new Promise(resolve => setTimeout(resolve, 1500));
        }
    }

    console.log("=== Auncient ROOTED Node.js Browser Controller ===");
    console.log(`Target URL: ${url}`);

    // Initialize WinchesterMQ connection if anvil is running
    try {
        const evmStart = performance.now();
        provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
        provider.pollingInterval = 20; // Reduce polling interval from default 4000ms to 20ms for instant input routing
        // Quick check to see if provider is online
        await provider.getNetwork();
        signer = await provider.getSigner(isYouTube ? 2 : 1);
        nextNonce = await provider.getTransactionCount(signer.address, "pending");
        const connTime = performance.now() - evmStart;
        console.log(`[WMQ] Connected to local EVM provider. Signer index: ${isYouTube ? 2 : 1} | Address: ${signer.address} | Initial Nonce: ${nextNonce} (took ${connTime.toFixed(3)} ms)`);
        
        // EVM diagnostics check
        const balStart = performance.now();
        const balance = await provider.getBalance(signer.address);
        const balTime = performance.now() - balStart;
        console.log(`[DIAGNOSTIC] Local EVM online. Signer Balance: ${ethers.formatEther(balance)} ETH (took ${balTime.toFixed(3)} ms)`);
        
        const fs = require('fs');
        const yulPath = path.join(__dirname, "../solidity/bin/WinchesterMQ.yul");
        if (fs.existsSync(yulPath)) {
            const wmqBytecode = compileYul(yulPath);
            let wmqAddress = null;
            let factoryDeployed = false;

            // Reuse existing WinchesterMQ contract if already deployed on Anvil
            const tempAddressPath = path.join(__dirname, "../tmp/wmq_address.txt");
            if (fs.existsSync(tempAddressPath)) {
                const savedAddress = fs.readFileSync(tempAddressPath, "utf8").trim();
                if (ethers.isAddress(savedAddress)) {
                    const code = await provider.getCode(savedAddress);
                    if (code !== "0x") {
                        console.log(`[WMQ] Reusing existing WinchesterMQ contract at: ${savedAddress}`);
                        wmqAddress = savedAddress;
                        wmqContract = new ethers.Contract(wmqAddress, [], signer);
                        factoryDeployed = true;
                    }
                }
            }

            // Attempt to deploy WinchesterMQ using the VerboseImmutableFactory if config is available
            try {
                const configPath = path.join(__dirname, "../config/user_config.json");
                if (fs.existsSync(configPath)) {
                    const config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
                    const factoryAddress = config.networks.localhost.immutableFactoryAddress;
                    if (factoryAddress) {
                        const bytecodeHash = ethers.keccak256(wmqBytecode);
                        const salt = ethers.id("WinchesterMQ");
                        const predictedAddress = ethers.getCreate2Address(factoryAddress, salt, bytecodeHash);

                        // Check if factory is running and check code at predicted address
                        const factoryCode = await provider.getCode(factoryAddress);
                        if (factoryCode !== "0x") {
                            const existingCode = await provider.getCode(predictedAddress);
                            if (existingCode !== "0x") {
                                console.log(`[WMQ] WinchesterMQ already deployed via factory at: ${predictedAddress}`);
                                wmqAddress = predictedAddress;
                                wmqContract = new ethers.Contract(wmqAddress, [], signer);
                                factoryDeployed = true;
                            } else {
                                console.log(`[WMQ] Deploying WinchesterMQ via Auncient VerboseImmutableFactory to: ${predictedAddress}`);
                                const masterKeyWallet = new ethers.Wallet(
                                    "0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690d"
                                ).connect(provider);
                                const signature = masterKeyWallet.signingKey.sign(bytecodeHash);
                                const vLeftAligned = ethers.concat([ethers.toBeHex(signature.v), ethers.zeroPadValue("0x", 31)]);
                                const masterKeyLeftAligned = ethers.concat([masterKeyWallet.address, ethers.zeroPadValue("0x", 12)]);

                                // createAuthorized(bytes32 salt, bytes32 r, bytes32 s, uint8 v, address masterKey) + bytecode
                                const createCalldata = ethers.concat([
                                    "0xb5ba0c68",
                                    salt,
                                    signature.r,
                                    signature.s,
                                    vLeftAligned,
                                    masterKeyLeftAligned,
                                    wmqBytecode
                                ]);

                                const deployTx = await signer.sendTransaction({
                                    to: factoryAddress,
                                    data: createCalldata,
                                    gasLimit: 25000000
                                });
                                await deployTx.wait();
                                console.log(`[WMQ] WinchesterMQ successfully deployed via factory at: ${predictedAddress}`);
                                wmqAddress = predictedAddress;
                                wmqContract = new ethers.Contract(wmqAddress, [], signer);
                                factoryDeployed = true;
                            }
                        }
                    }
                }
            } catch (factoryErr) {
                console.log(`[WMQ] Factory deployment attempt failed, falling back to direct deploy: ${factoryErr.message}`);
            }

            if (!factoryDeployed) {
                const wmqFactory = new ethers.ContractFactory([], wmqBytecode, signer);
                wmqContract = await wmqFactory.deploy();
                await wmqContract.waitForDeployment();
                wmqAddress = await wmqContract.getAddress();
                console.log(`[WMQ] WinchesterMQ successfully deployed directly at: ${wmqAddress}`);
            }

            wmqAddressStr = wmqAddress;

            // Write address to file for backend server reference
            try {
                fs.writeFileSync(path.join(__dirname, "../tmp/wmq_address.txt"), wmqAddress);
            } catch (err) {}

            // Synchronize nonce after deployment transactions
            nextNonce = await provider.getTransactionCount(signer.address, "pending");

            // Auncient high-frequency JSON-RPC block log poller for absolute input reliability
            let lastPolledBlock = await provider.getBlockNumber();
            const seenLogs = new Set();

            async function pollLogs() {
                try {
                    const latestBlock = await provider.getBlockNumber();
                    if (latestBlock >= lastPolledBlock) {
                        const logs = await provider.getLogs({
                            address: wmqAddress,
                            fromBlock: lastPolledBlock,
                            toBlock: latestBlock
                        });

                        for (const log of logs) {
                            const logKey = `${log.blockNumber}:${log.transactionHash}:${log.index}`;
                            if (seenLogs.has(logKey)) continue;
                            seenLogs.add(logKey);

                            // Bounded cache cleanup to prevent memory leaks
                            if (seenLogs.size > 2000) {
                                const iter = seenLogs.values();
                                for (let k = 0; k < 500; k++) {
                                    seenLogs.delete(iter.next().value);
                                }
                            }

                            if (log.topics[0] === "0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29a1bee1dae9af77dac73aa045") {
                                // Handshake Mode: LogPut
                                try {
                                    const data = ethers.getBytes(log.data);
                                    const blockIdBytes = data.slice(32, 64);
                                    const blockId = ethers.toBigInt(blockIdBytes);

                                    let blockBytes = [];
                                    const baseKey = ethers.keccak256(ethers.zeroPadValue(ethers.toBeHex(BigInt(0x1000) + blockId), 32));
                                    for (let i = 0; i < 8; i++) {
                                        const slotKey = ethers.toBeHex(BigInt(baseKey) + BigInt(i), 32);
                                        const slotVal = await provider.getStorage(wmqAddress, slotKey);
                                        blockBytes.push(...ethers.getBytes(slotVal));
                                    }
                                    const fullStr = Buffer.from(blockBytes).toString('utf8');
                                    const commandStr = fullStr.split('\0')[0].trim();
                                    const targetPrefix = isYouTube ? "Y:" : "M:";
                                    if (commandStr && commandStr.startsWith(targetPrefix)) {
                                        wmqEventCount++;
                                        let actualCmd = commandStr.substring(targetPrefix.length);
                                        // Expand abbreviated command names back to full names
                                        actualCmd = actualCmd
                                            .replace("MM ", "MOUSE_MOVE ")
                                            .replace("MD ", "MOUSE_DOWN ")
                                            .replace("MU ", "MOUSE_UP ")
                                            .replace("MS ", "MOUSE_SCROLL ")
                                            .replace("KD ", "KEY_DOWN ")
                                            .replace("KU ", "KEY_UP ");
                                        console.log(`[WinchesterMQ Event Log] Routed input to ${isYouTube ? 'YouTube' : 'Main'}: ${actualCmd}`);
                                        await handleInputCommand(actualCmd);
                                    }
                                } catch (err) {
                                    console.error("[WMQ Listener ERR] Failed to parse input command block:", err);
                                }
                            } else if (log.topics[0] === "0xe1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1da") {
                                // Fast Path Mode: postEvent
                                try {
                                    const commandStr = Buffer.from(ethers.getBytes(log.data)).toString('utf8').replace(/\0/g, '').trim();
                                    const targetPrefix = isYouTube ? "Y:" : "M:";
                                    if (commandStr && commandStr.startsWith(targetPrefix)) {
                                        wmqEventCount++;
                                        let actualCmd = commandStr.substring(targetPrefix.length);
                                        // Expand abbreviated command names back to full names
                                        actualCmd = actualCmd
                                            .replace("MM ", "MOUSE_MOVE ")
                                            .replace("MD ", "MOUSE_DOWN ")
                                            .replace("MU ", "MOUSE_UP ")
                                            .replace("MS ", "MOUSE_SCROLL ")
                                            .replace("KD ", "KEY_DOWN ")
                                            .replace("KU ", "KEY_UP ");
                                        console.log(`[WinchesterMQ Fast Log] Routed input to ${isYouTube ? 'YouTube' : 'Main'}: ${actualCmd}`);
                                        await handleInputCommand(actualCmd);
                                    }
                                } catch (err) {
                                    console.error("[WMQ Fast Listener ERR] Failed to parse command:", err);
                                }
                            }
                        }
                        lastPolledBlock = latestBlock;
                    }
                } catch (err) {
                    console.error("[WMQ Poller ERR]", err);
                }
                setTimeout(pollLogs, 30);
            }
            pollLogs();
        }
    } catch (e) {
        console.log("[WMQ] WinchesterMQ not active or unreachable (Anvil offline). Using standard direct routing.");
    }

    // 1. Launch presenter process if not running in YouTube sub-browser frame-dumper mode
    let presenter = null;
    let presenterReadyResolver;
    const presenterReady = new Promise((resolve) => {
        presenterReadyResolver = resolve;
    });

    if (!isYouTube) {
        const presenterPath = path.join(__dirname, "../tsfi2-deepseek/tests/rooted_frame_presenter");
        console.log(`[PRESENTER] Spawning ${presenterPath}`);
        presenter = spawn(presenterPath, [], {
            env: {
                ...process.env,
                WAYLAND_DISPLAY: process.env.WAYLAND_DISPLAY || "wayland-1"
            }
        });

        presenter.stderr.on("data", data => {
            console.error(`[PRESENTER ERR] ${data.toString().trim()}`);
        });
    } else {
        // Resolve immediately for headless dumper
        setTimeout(() => {
            if (presenterReadyResolver) presenterReadyResolver();
        }, 100);
    }

    // Define helper function to route input events to Puppeteer
    let page = null;
    let isResizing = false;
    let videoBounds = { x: 427, y: 221, width: 341, height: 145 };
    let lastBoundsUpdateTime = 0;
    async function updateVideoBounds() {
        if (!page) return;
        lastBoundsUpdateTime = Date.now();
        try {
            for (const frame of page.frames()) {
                try {
                    const rect = await frame.evaluate(() => {
                        const el = document.getElementById('browserStreamSvg');
                        if (!el) return null;
                        const r = el.getBoundingClientRect();
                        return { x: Math.round(r.x), y: Math.round(r.y), width: Math.round(r.width), height: Math.round(r.height) };
                    });
                    
                    if (rect && rect.width > 0 && rect.height > 0) {
                        let offsetX = 0;
                        let offsetY = 0;
                        
                        if (frame !== page.mainFrame()) {
                            const parentFrame = frame.parentFrame();
                            if (parentFrame) {
                                const frameUrl = frame.url();
                                const offset = await parentFrame.evaluate((url) => {
                                    const iframes = document.getElementsByTagName('iframe');
                                    for (const iframe of iframes) {
                                        if (iframe.src.includes(url) || (iframe.contentWindow && iframe.contentWindow.location.href.includes(url))) {
                                            const r = iframe.getBoundingClientRect();
                                            return { x: Math.round(r.x), y: Math.round(r.y) };
                                        }
                                    }
                                    return null;
                                }, frameUrl);
                                
                                if (offset) {
                                    offsetX = offset.x;
                                    offsetY = offset.y;
                                }
                            }
                        }
                        
                        videoBounds = {
                            x: rect.x + offsetX,
                            y: rect.y + offsetY,
                            width: rect.width,
                            height: rect.height
                        };
                        return; // Found and updated bounds successfully
                    }
                } catch (frameErr) {}
            }
        } catch (err) {}
    }
    let controlDown = false;
    let shiftDown = false;
    const shiftMap = {
        '1': '!', '2': '@', '3': '#', '4': '$', '5': '%', '6': '^', '7': '&', '8': '*', '9': '(', '0': ')',
        '-': '_', '=': '+', '[': '{', ']': '}', ';': ':', "'": '"', '`': '~', '\\': '|', ',': '<', '.': '>', '/': '?'
    };
    let lastClickTime = 0;
    let clickCount = 1;
    let resizeTimeout = null;
    let lastMouseX = 0;
    let lastMouseY = 0;
    let lastMouseDownX = 0;
    let lastMouseDownY = 0;
    let lastMouseDownTime = 0;
    let isFocusedOnYouTube = false;

    const timingTelemetry = {
        MOUSE_MOVE: { count: 0, totalMs: 0, maxMs: 0 },
        MOUSE_DOWN: { count: 0, totalMs: 0, maxMs: 0 },
        MOUSE_UP: { count: 0, totalMs: 0, maxMs: 0 },
        MOUSE_SCROLL: { count: 0, totalMs: 0, maxMs: 0 },
        KEY_DOWN: { count: 0, totalMs: 0, maxMs: 0 },
        KEY_UP: { count: 0, totalMs: 0, maxMs: 0 },
        WINDOW_RESIZE: { count: 0, totalMs: 0, maxMs: 0 }
    };

    async function processQueue() {
        if (processingQueue || inputQueue.length === 0) return;
        processingQueue = true;
        while (inputQueue.length > 0) {
            let item = inputQueue.shift();
            if (item.line.startsWith('MOUSE_MOVE')) {
                while (inputQueue.length > 0 && inputQueue[0].line.startsWith('MOUSE_MOVE')) {
                    if (item.callback) item.callback();
                    item = inputQueue.shift();
                }
            }
            try {
                if (item.line.trim() === 'PING') {
                    if (item.callback) item.callback();
                } else {
                    await executeInputCommand(item.line);
                    if (item.callback) item.callback();
                }
            } catch (err) {
                console.error("[INPUT QUEUE ERR] Error executing input command:", item.line, err);
            }
        }
        processingQueue = false;
    }

    function handleInputCommand(line, callback) {
        inputQueue.push({ line, callback });
        processQueue().catch(() => {});
    }

    async function executeInputCommand(line) {
        const startHrTime = process.hrtime.bigint();
        line = line.replace(/,/g, ' ');
        if (line.includes("Streaming starting...")) {
            console.log(`[PRESENTER OUT] ${line}`);
            if (presenterReadyResolver) {
                presenterReadyResolver();
            }
            return;
        }
        // Normalize commas to spaces for Auncient token compatibility, then split by whitespace
        const parts = line.replace(/,/g, ' ').split(/\s+/).filter(Boolean);
        if (parts.length === 0) return;
        const cmd = parts[0];
        
        try {
            if (cmd === 'MOUSE_MOVE') {
                const x = parseInt(parts[1]);
                const y = parseInt(parts[2]);
                lastMouseX = x;
                lastMouseY = y;
                
                if (isYouTube) {
                    await page.mouse.move(x, y);
                    // Manually trigger mousemove event on YouTube player container to force controls to pop up
                    await page.evaluate(() => {
                        const player = document.querySelector('.html5-video-player, video');
                        if (player) {
                            const event = new MouseEvent('mousemove', {
                                bubbles: true,
                                cancelable: true,
                                view: window
                            });
                            player.dispatchEvent(event);
                        }
                    }).catch(() => {});
                } else {
                    await page.mouse.move(x, y);
                }
            } else if (cmd === 'MOUSE_DOWN') {
                const btn = parseInt(parts[1]);
                const button = linuxButtonMap[btn] || 'left';
                let x = lastMouseX;
                let y = lastMouseY;
                if (parts[2] !== undefined && parts[3] !== undefined) {
                    x = parseInt(parts[2]);
                    y = parseInt(parts[3]);
                    lastMouseX = x;
                    lastMouseY = y;
                    await page.mouse.move(x, y);
                }
                lastMouseDownX = x;
                lastMouseDownY = y;
                lastMouseDownTime = Date.now();
                
                if (isYouTube) {
                    // Executed directly on sub-browser page
                } else {
                    const inVideoBounds = x >= videoBounds.x && x <= videoBounds.x + videoBounds.width &&
                                         y >= videoBounds.y && y <= videoBounds.y + videoBounds.height;
                    isFocusedOnYouTube = inVideoBounds;
                    console.log(`[INPUT ROUTER] MOUSE_DOWN at (${x}, ${y}). bounds={x:${videoBounds.x}, y:${videoBounds.y}, w:${videoBounds.width}, h:${videoBounds.height}}. inVideoBounds=${inVideoBounds}. isFocusedOnYouTube=${isFocusedOnYouTube}`);
                    if (inVideoBounds) {
                        const x_yt = Math.round(((x - videoBounds.x) / videoBounds.width) * 800);
                        const y_yt = Math.round(((y - videoBounds.y) / videoBounds.height) * 600);
                        sendWmqEvent('YOUTUBE:MOUSE_DOWN', `${btn},${x_yt},${y_yt}`).catch(() => {});
                    }
                }
                
                const now = Date.now();
                if (now - lastClickTime < 300) {
                    clickCount++;
                } else {
                    clickCount = 1;
                }
                lastClickTime = now;
                await page.mouse.down({ button, clickCount });
            } else if (cmd === 'MOUSE_UP') {
                const btn = parseInt(parts[1]);
                const button = linuxButtonMap[btn] || 'left';
                let x = lastMouseX;
                let y = lastMouseY;
                if (parts[2] !== undefined && parts[3] !== undefined) {
                    x = parseInt(parts[2]);
                    y = parseInt(parts[3]);
                    lastMouseX = x;
                    lastMouseY = y;
                    await page.mouse.move(x, y);
                }
                
                if (isYouTube) {
                    // Executed directly on sub-browser page
                } else {
                    const inVideoBounds = x >= videoBounds.x && x <= videoBounds.x + videoBounds.width &&
                                         y >= videoBounds.y && y <= videoBounds.y + videoBounds.height;
                    if (inVideoBounds) {
                        const x_yt = Math.round(((x - videoBounds.x) / videoBounds.width) * 800);
                        const y_yt = Math.round(((y - videoBounds.y) / videoBounds.height) * 600);
                        sendWmqEvent('YOUTUBE:MOUSE_UP', `${btn},${x_yt},${y_yt}`).catch(() => {});
                    }
                }
                
                const isClick = (Date.now() - lastMouseDownTime < 350) &&
                                (Math.abs(x - lastMouseDownX) < 10) &&
                                (Math.abs(y - lastMouseDownY) < 10);
                if (isClick) {
                    // Snap back to initial down coordinates to guarantee click firing in Puppeteer
                    await page.mouse.move(lastMouseDownX, lastMouseDownY);
                }
                await page.mouse.up({ button, clickCount });
            } else if (cmd === 'KEY_DOWN') {
                const key = parseInt(parts[1]);
                const keyName = linuxKeyMap[key];
                
                if (isYouTube) {
                    console.log(`[YOUTUBE INPUT] KEY_DOWN: key=${key}, keyName=${keyName}`);
                    if (keyName === 'Shift') {
                        shiftDown = true;
                    }
                    if (keyName === 'Control') {
                        controlDown = true;
                    }
                    if (keyName) {
                        await page.keyboard.down(keyName);
                    }
                } else {
                    console.log(`[INPUT ROUTER] KEY_DOWN: key=${key}, keyName=${keyName}, isFocusedOnYouTube=${isFocusedOnYouTube}`);
                    if (isFocusedOnYouTube) {
                        sendWmqEvent('YOUTUBE:KEY_DOWN', `${key}`).catch(() => {});
                    } else {
                        if (keyName === 'Control') {
                            controlDown = true;
                        }
                        if (keyName === 'r' && controlDown) {
                            console.log("[PUPPETEER] Control+R detected. Reloading page...");
                            await page.reload({ waitUntil: "networkidle2" });
                        } else if (keyName) {
                            if (keyName === 'Enter') {
                                await page.keyboard.press('Enter');
                            } else {
                                await page.keyboard.down(keyName);
                            }
                        }
                    }
                }
            } else if (cmd === 'KEY_UP') {
                const key = parseInt(parts[1]);
                const keyName = linuxKeyMap[key];
                
                if (isYouTube) {
                    if (keyName === 'Shift') {
                        shiftDown = false;
                    }
                    if (keyName === 'Control') {
                        controlDown = false;
                    }
                    if (keyName) {
                        await page.keyboard.up(keyName);
                    }
                } else {
                    if (isFocusedOnYouTube) {
                        sendWmqEvent('YOUTUBE:KEY_UP', `${key}`).catch(() => {});
                    } else {
                        if (keyName === 'Control') {
                            controlDown = false;
                        }
                        if (keyName && keyName !== 'Enter') {
                            await page.keyboard.up(keyName);
                        }
                    }
                }
            } else if (cmd === 'MOUSE_SCROLL') {
                const axis = parseInt(parts[1]);
                const value = parseFloat(parts[2]);
                
                let x = lastMouseX;
                let y = lastMouseY;
                if (isYouTube && parts[3] !== undefined && parts[4] !== undefined) {
                    x = parseInt(parts[3]);
                    y = parseInt(parts[4]);
                    lastMouseX = x;
                    lastMouseY = y;
                }
                
                console.log(`[INPUT ROUTER] Processed MOUSE_SCROLL event: axis=${axis}, value=${value}, x=${x}, y=${y}, target=${isYouTube ? 'YouTube' : 'Main'}`);
                
                const multiplier = isYouTube ? 55 : 25;
                const deltaY = (axis === 0) ? value * multiplier : 0;
                const deltaX = (axis === 1) ? value * multiplier : 0;
                
                if (isYouTube) {
                    // Auncient direct scroll path for YouTube: highly customized containers cancel/ignore native wheel events
                    await page.evaluate((x, y, dx, dy) => {
                        const el = document.elementFromPoint(x, y) || document.body;
                        const ev = new WheelEvent('wheel', {
                            bubbles: true,
                            cancelable: true,
                            clientX: x,
                            clientY: y,
                            deltaX: dx,
                            deltaY: dy,
                            deltaMode: 0
                        });
                        el.dispatchEvent(ev);
                        
                        let parent = el;
                        let scrolled = false;
                        while (parent && parent !== document.body && parent !== document.documentElement) {
                            if (parent.scrollHeight > parent.clientHeight || parent.scrollWidth > parent.clientWidth) {
                                const style = window.getComputedStyle(parent);
                                if (style.overflowY === 'auto' || style.overflowY === 'scroll' || style.overflowX === 'auto' || style.overflowX === 'scroll' || parent.id === 'page-manager' || parent.tagName === 'YTD-APP') {
                                    parent.scrollTop += dy;
                                    parent.scrollLeft += dx;
                                    scrolled = true;
                                    break;
                                }
                            }
                            parent = parent.parentElement;
                        }
                        if (!scrolled) {
                            window.scrollBy(dx, dy);
                        }
                    }, x, y, deltaX, deltaY).catch(() => {});
                } else {
                    const inVideoBounds = lastMouseX >= videoBounds.x && lastMouseX <= videoBounds.x + videoBounds.width &&
                                         lastMouseY >= videoBounds.y && lastMouseY <= videoBounds.y + videoBounds.height;
                    if (inVideoBounds) {
                        const x_yt = Math.round(((lastMouseX - videoBounds.x) / videoBounds.width) * 800);
                        const y_yt = Math.round(((lastMouseY - videoBounds.y) / videoBounds.height) * 600);
                        sendWmqEvent('YOUTUBE:MOUSE_SCROLL', `${axis},${value},${x_yt},${y_yt}`).catch(() => {});
                    } else {
                        try {
                            await page.mouse.wheel({ deltaX, deltaY });
                        } catch (wheelErr) {
                            await page.evaluate((x, y, dx, dy) => {
                                const el = document.elementFromPoint(x, y) || document.body;
                                const ev = new WheelEvent('wheel', {
                                    bubbles: true,
                                    cancelable: true,
                                    clientX: x,
                                    clientY: y,
                                    deltaX: dx,
                                    deltaY: dy,
                                    deltaMode: 0
                                });
                                el.dispatchEvent(ev);
                                if (el === document.body || el === document.documentElement || el.id === 'page-manager') {
                                    window.scrollBy(dx, dy);
                                } else {
                                    let parent = el;
                                    while (parent && parent !== document.body) {
                                        if (parent.scrollHeight > parent.clientHeight || parent.scrollWidth > parent.clientWidth) {
                                            parent.scrollTop += dy;
                                            parent.scrollLeft += dx;
                                            break;
                                        }
                                        parent = parent.parentElement;
                                    }
                                }
                            }, lastMouseX, lastMouseY, deltaX, deltaY).catch(() => {});
                        }
                    }
                }
            } else if (cmd === 'WINDOW_CLOSE') {
                console.log("[PUPPETEER] Wayland close event requested. Shutting down browser...");
                active = false;
                try {
                    await page.setMuted(true);
                    await page.evaluate(() => {
                        document.querySelectorAll('video, audio').forEach(el => {
                            el.pause();
                            el.src = "";
                        });
                    });
                } catch (e) {}
                await browser.close();
                process.exit(0);
            } else if (cmd === 'WINDOW_RESIZE') {
                const width = parseInt(parts[1]);
                const height = parseInt(parts[2]);
                presenterWidth = width;
                presenterHeight = height;
                if (resizeTimeout) {
                    clearTimeout(resizeTimeout);
                }
                resizeTimeout = setTimeout(async () => {
                    console.log(`[PUPPETEER] Resizing viewport to ${width}x${height}`);
                    try {
                        isResizing = true;
                        // Auncient synchronization sequence: stop screencast to prevent race condition crashes
                        if (client) {
                            try {
                                await client.send('Page.stopScreencast');
                            } catch (e) {}
                        }
                        await page.setViewport({ width, height });
                        await updateVideoBounds();
                        // Restart screencast now that viewport layout has stabilized
                        if (client) {
                            try {
                                await client.send('Page.startScreencast', { format: 'jpeg', quality: 80 });
                            } catch (e) {}
                        }
                        isResizing = false;
                    } catch (viewportErr) {
                        isResizing = false;
                        // ignore viewport errors
                    }
                }, 150);
            } else if (line.startsWith('search ')) {
                const queryText = line.substring(7).trim();
                console.log(`[PUPPETEER] Received search command for query: "${queryText}"`);
                const searchUrl = `https://www.youtube.com/results?search_query=${encodeURIComponent(queryText)}`;
                await page.goto(searchUrl, { waitUntil: "networkidle2" });
                console.log(`[PUPPETEER] Navigated to search results for: "${queryText}"`);
            } else {
                console.log(`[PRESENTER OUT] ${line}`);
            }
        } catch (err) {
            // Suppress error if Puppeteer is closed or navigating
        }

        // Record timing telemetry for input command execution
        const endHrTime = process.hrtime.bigint();
        const durationMs = Number(endHrTime - startHrTime) / 1e6;
        if (timingTelemetry[cmd]) {
            const data = timingTelemetry[cmd];
            data.count++;
            data.totalMs += durationMs;
            if (durationMs > data.maxMs) {
                data.maxMs = durationMs;
            }

            const totalEvents = Object.values(timingTelemetry).reduce((sum, item) => sum + item.count, 0);
            if (totalEvents % 50 === 0) {
                console.log("=== Auncient Input Command Processing Timing Telemetry ===");
                for (const [evt, item] of Object.entries(timingTelemetry)) {
                    if (item.count > 0) {
                        const avg = (item.totalMs / item.count).toFixed(3);
                        console.log(`  |- [TELEMETRY] ${evt}: count=${item.count}, avg=${avg}ms, max=${item.maxMs.toFixed(3)}ms`);
                    }
                }
                console.log("==========================================================");
            }
        }
    }

    // 1. Read input events from presenter stdout (hooked early) if presenter exists
    if (presenter) {
        const rl = readline.createInterface({
            input: presenter.stdout,
            terminal: false
        });
        rl.on('line', handleInputCommand);
    }

    // 2. Named pipe setup removed as per WinchesterMQ-only routing rule. All inputs flow through the Auncient VM/WMQ.

    console.log("[PRESENTER] Waiting for presenter to initialize...");
    await presenterReady;
    console.log("[PRESENTER] Presenter is ready. Starting Puppeteer...");

    // 2. Launch Puppeteer
    console.log("[PUPPETEER] Launching system Google Chrome...");
    const browser = await puppeteer.launch({
        executablePath: "/usr/bin/google-chrome",
        headless: true,
        dumpio: true,
        ignoreDefaultArgs: ['--mute-audio'],
        args: [
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--window-size=800,600",
            "--autoplay-policy=no-user-gesture-required",
            "--disable-blink-features=AutomationControlled",
            "--disable-accelerated-video-decode",
            `--user-data-dir=${path.join(__dirname, "../tmp/puppeteer_chrome_profile_" + Date.now())}`
        ]
    });
    
    async function cleanup() {
        console.log("[PUPPETEER] SIGINT/SIGTERM received. Cleaning up and closing browser...");
        active = false;
        try {
            await browser.close();
        } catch (err) {}
        process.exit(0);
    }
    process.on('SIGINT', cleanup);
    process.on('SIGTERM', cleanup);

    page = await browser.newPage();
    await page.evaluateOnNewDocument(() => {
        try {
            delete navigator.__proto__.webdriver;
        } catch (e) {}
    });
    page.on('pageerror', err => {
        console.error('[PUPPETEER PAGE ERROR]', err.stack || err.message);
    });
    page.on('error', err => {
        console.error('[PUPPETEER PAGE CRASH]', err.message);
    });
    page.on('console', msg => {
        console.log('[PUPPETEER CONSOLE]', msg.text());
    });
    page.on('requestfailed', request => {
        const failure = request.failure();
        const errorText = failure ? failure.errorText : 'unknown';
        const type = request.resourceType();
        if (type === 'media' || type === 'xhr' || type === 'fetch' || errorText.includes('ERR_')) {
            console.error(`[PUPPETEER REQUEST FAILED] Type: ${type} | URL: ${request.url()} | Error: ${errorText}`);
        }
    });
    await page.setUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36 ROOTED Browser/1.0");
    await page.setViewport({ width: 1024, height: 768 });

    let active = true;
    if (presenter) {
        presenter.on("exit", (code, signal) => {
            console.log(`[PRESENTER] Exited with code=${code}, signal=${signal}. Presenter window closed, keeping browser active for dashboard.`);
        });
    }

    // Screenshot streaming loop will be declared and run after page load to prevent early crashes.
    
    if (isYouTube) {
        setInterval(() => {
            http.get({
                hostname: '127.0.0.1',
                port: 8000,
                path: '/api/has-mjpeg-clients',
                agent: keepAliveAgent
            }, (res) => {
                let data = '';
                res.on('data', chunk => data += chunk);
                res.on('end', () => {
                    try {
                        const parsed = JSON.parse(data);
                        hasMjpegClients = !!parsed.hasClients;
                    } catch (e) {}
                });
            }).on('error', () => {
                hasMjpegClients = false;
            });
        }, 2000);
    }

    let lastReloadTime = 0;
    // Auto-unmute and auto-recover YouTube video player continuously in the background
    setInterval(async () => {
        try {
            if (!isYouTube) {
                const currentUrl = page.url();
                const isLoaded = await page.evaluate((currUrl) => {
                    if (currUrl.includes("atropa_splash.html")) {
                        return !!document.getElementById("bearCanvas") || !!document.getElementById("emblemContainer");
                    }
                    if (currUrl.includes("hub_portal.html")) {
                        return !!document.querySelector(".portal-layout") || !!document.querySelector(".portal-tabs");
                    }
                    return !!document.getElementById("vulkanPipelineCanvas") || !!document.getElementById("matrixCanvas");
                }, currentUrl).catch(() => false);
                if (!isLoaded) {
                    console.log("[PUPPETEER] Dashboard UI or Splash not detected. Reconnecting to " + url + "...");
                    await page.goto(url, { waitUntil: "networkidle2" }).catch(() => {});
                }
                return;
            }
            const status = await page.evaluate(() => {
                // 1. Auto unmute
                const muteBtn = document.querySelector('.ytp-mute-button');
                if (muteBtn) {
                    const titleText = (muteBtn.getAttribute('title') || muteBtn.getAttribute('aria-label') || '').toLowerCase();
                    if (titleText.includes('unmute')) {
                        muteBtn.click();
                    }
                }
                
                // 2. Click retry button if visible
                const retryBtn = document.querySelector('.ytp-error-message-button, .ytp-retry-button, button[aria-label="Retry"]');
                if (retryBtn) {
                    retryBtn.click();
                }

                // 3. Auto skip ads
                const skipBtn = document.querySelector('.ytp-skip-ad-button, .ytp-ad-skip-button-modern, .ytp-skip-ad-button-text, .ytp-ad-skip-button');
                if (skipBtn) {
                    skipBtn.click();
                }
                const closeAdBtn = document.querySelector('.ytp-ad-overlay-close-button');
                if (closeAdBtn) {
                    closeAdBtn.click();
                }

                // Check if playback error overlay is active
                const errScreen = document.querySelector('.ytp-error, .ytp-playability-error-supported-renderers, #error-screen');
                const hasError = !!(errScreen && errScreen.offsetWidth > 0 && errScreen.offsetHeight > 0) || (document.body.innerText || "").includes("Something went wrong");
                const errorText = document.querySelector('.ytp-error-message-text')?.innerText || "";
                const errorSubtext = document.querySelector('.ytp-error-message-subtext')?.innerText || "";

                const video = document.querySelector('video');
                let videoError = null;
                if (video && video.error) {
                    videoError = {
                        code: video.error.code,
                        message: video.error.message
                    };
                }

                let playerState = null;
                const ytPlayer = document.querySelector('.html5-video-player');
                if (ytPlayer && typeof ytPlayer.getPlayerState === 'function') {
                    playerState = ytPlayer.getPlayerState();
                }

                // Extra telemetry parameters
                const adActive = document.querySelector('.ad-showing, .html5-ad-product-override') !== null;
                const videoQuality = (ytPlayer && typeof ytPlayer.getPlaybackQuality === 'function') ? ytPlayer.getPlaybackQuality() : 'unknown';
                const volume = video.volume;
                const playbackRate = video.playbackRate;
                const bufferedPercent = video.duration > 0 && video.buffered.length > 0 ? 
                    (video.buffered.end(video.buffered.length - 1) / video.duration) * 100 : 0;

                // Get page scroll position
                const scrollY = window.scrollY || document.documentElement.scrollTop;
                const container = document.querySelector('#content, #page-manager, ytd-app, #primary');
                const containerScrollTop = container ? container.scrollTop : 0;

                return {
                    found: true,
                    currentTime: video.currentTime,
                    duration: video.duration,
                    paused: video.paused,
                    muted: video.muted,
                    title: document.title,
                    hasError,
                    errorText,
                    errorSubtext,
                    videoError,
                    playerState,
                    ended: video.ended || playerState === 0,
                    adActive,
                    videoQuality,
                    volume,
                    playbackRate,
                    bufferedPercent,
                    scrollY,
                    containerScrollTop
                };
            });

            if (provider) {
                try {
                    lastBlockNumber = await provider.getBlockNumber();
                } catch (bErr) {}
            }

            if (status.found) {
                const cur = (typeof status.currentTime === 'number') ? status.currentTime.toFixed(1) : '0';
                const dur = (typeof status.duration === 'number') ? status.duration.toFixed(1) : '0';
                console.log(`[VIDEO STATUS] Time: ${cur}s / ${dur}s, Paused: ${status.paused}, Muted: ${status.muted}, ScrollY: ${status.scrollY}, ContainerScrollTop: ${status.containerScrollTop}, Error: ${status.hasError} (${status.errorText}), Title: "${status.title}"`);
                await sendWmqEvent("VIDEO_STATUS", `${cur},${dur},${status.paused},${status.muted}`);
                
                try {
                    const http = require('http');
                    const postData = JSON.stringify({
                        currentTime: parseFloat(cur),
                        duration: parseFloat(dur),
                        paused: status.paused,
                        muted: status.muted,
                        hasError: status.hasError || !!status.videoError,
                        errorText: status.errorText || (status.videoError ? status.videoError.message : ""),
                        errorSubtext: status.errorSubtext || "",
                        playerState: status.playerState,
                        ended: status.ended,
                        wmqAddress: wmqAddressStr,
                        wmqEventCount: wmqEventCount,
                        blockNumber: lastBlockNumber,
                        presenterWidth: presenterWidth,
                        presenterHeight: presenterHeight,
                        title: status.title,
                        adActive: status.adActive,
                        videoQuality: status.videoQuality,
                        volume: status.volume,
                        playbackRate: status.playbackRate,
                        bufferedPercent: status.bufferedPercent
                    });
                    const req = http.request({
                        hostname: '127.0.0.1',
                        port: 8000,
                        path: '/update-video-status',
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                            'Content-Length': Buffer.byteLength(postData)
                        }
                    }, (res) => {
                        res.resume();
                    });
                    req.on('error', () => {});
                    req.write(postData);
                    req.end();
                } catch (postErr) {}
            }

            if (status.hasError || status.videoError) {
                console.log(`[VIDEO ERROR DETECTED] ErrorScreen: ${status.hasError}, ErrorText: "${status.errorText}", ErrorSubtext: "${status.errorSubtext}", VideoError: ${JSON.stringify(status.videoError)}, PlayerState: ${status.playerState}`);
                const now = Date.now();
                if (now - lastReloadTime > 15000) {
                    console.log("[PUPPETEER] Triggering page reload due to video error...");
                    lastReloadTime = now;
                    await page.reload({ waitUntil: "networkidle2" });
                }
            }
        } catch (e) {
            console.log(`[MONITOR ERR] ${e.message}`);
            if (e.message.includes("closed") || e.message.includes("destroyed") || e.message.includes("crashed")) {
                try {
                    console.log("[PUPPETEER] Attempting to recover page after crash...");
                    await page.reload({ waitUntil: "networkidle2" });
                } catch (re) {
                    console.log("[PUPPETEER] Recovery failed: " + re.message);
                }
            }
        }
    }, 1000);
    
    try {
        console.log(`[PUPPETEER] Navigating directly to target URL: ${url}...`);
        await page.goto(url, { waitUntil: "networkidle2" });
        console.log("[PUPPETEER] Navigation complete.");
    } catch (navErr) {
        console.log("[PUPPETEER] Initial navigation finished or timed out: " + navErr.message);
    }

    async function uploadFrame(jpegBuffer) {
        if (!isYouTube || !hasMjpegClients) return;
        return new Promise((resolve) => {
            const req = http.request({
                hostname: '127.0.0.1',
                port: 8000,
                path: '/upload-frame',
                method: 'POST',
                agent: keepAliveAgent,
                headers: {
                    'Content-Type': 'image/jpeg',
                    'Content-Length': jpegBuffer.length
                }
            }, (res) => {
                res.resume();
                resolve();
            });
            req.on('error', () => resolve());
            req.write(jpegBuffer);
            req.end();
        });
    }

    let frameCount = 0;
    let client = null;
    async function startScreencast() {
        console.log("[PUPPETEER] startScreencast() triggered. Target ID:", page.target()._targetId);
        try {
            if (client) {
                try {
                    console.log("[PUPPETEER] Detaching existing CDPSession client...");
                    await client.detach();
                } catch (detachErr) {
                    console.log("[PUPPETEER] Detach warning:", detachErr.message);
                }
                client = null;
            }
            client = await page.target().createCDPSession();
            console.log("[PUPPETEER] Created CDPSession client.");
            await client.send('Page.startScreencast', { format: 'jpeg', quality: 80 });
            console.log("[PUPPETEER] Page.startScreencast sent successfully.");
            client.on('Page.screencastFrame', async ({ data, metadata, sessionId }) => {
                try {
                    if (!active || isResizing) {
                        try {
                            await client.send('Page.screencastFrameAck', { sessionId });
                        } catch (e) {}
                        return;
                    }
                    const jpegBuffer = Buffer.from(data, 'base64');
                    frameCount++;
                    if (isYouTube) {
                        uploadFrame(jpegBuffer);
                    }
                    try {
                        const fs = require('fs');
                        const tmpPath = isYouTube ? "/dev/shm/atropa_youtube_frame.tmp" : "/dev/shm/atropa_latest_frame.tmp";
                        const targetPath = isYouTube ? "/dev/shm/atropa_youtube_frame.jpg" : "/dev/shm/atropa_latest_frame.jpg";
                        fs.writeFileSync(tmpPath, jpegBuffer);
                        fs.renameSync(tmpPath, targetPath);
                        if (!isYouTube) {
                            sendWmqEvent("M:FRAME", jpegBuffer.length);
                        }
                    } catch (writeErr) {
                        // ignore write errors
                    }

                    if (presenter && active && presenter.stdin.writable) {
                        const headerBuf = Buffer.alloc(20);
                        headerBuf.writeUInt32LE(jpegBuffer.length, 0);
                        headerBuf.writeInt32LE(videoBounds.x, 4);
                        headerBuf.writeInt32LE(videoBounds.y, 8);
                        headerBuf.writeInt32LE(videoBounds.width, 12);
                        headerBuf.writeInt32LE(videoBounds.height, 16);

                        let ok = presenter.stdin.write(headerBuf);
                        if (ok) {
                            ok = presenter.stdin.write(jpegBuffer);
                        }
                        if (!ok) {
                            await new Promise(resolve => presenter.stdin.once('drain', resolve));
                        }
                        if (frameCount % 180 === 0) {
                            console.log(`[STREAM] Sent ${frameCount} screencast frames.`);
                        }
                    }
                    try {
                        await client.send('Page.screencastFrameAck', { sessionId });
                    } catch (e) {}
                } catch (err) {
                    try {
                        await client.send('Page.screencastFrameAck', { sessionId });
                    } catch (ackErr) {}
                }
            });
        } catch (screencastErr) {
            console.error("[PUPPETEER] Screencast setup failed:", screencastErr);
            screenshotLoop();
        }
    }

    async function screenshotLoop() {
        while (active) {
            if (isResizing) {
                await new Promise(resolve => setTimeout(resolve, 50));
                continue;
            }
            try {
                const startTime = Date.now();
                const jpegBuffer = await page.screenshot({ type: 'jpeg', quality: 75 });
                frameCount++;
                if (isYouTube) {
                    uploadFrame(jpegBuffer);
                }
                try {
                    const fs = require('fs');
                    const tmpPath = isYouTube ? "/dev/shm/atropa_youtube_frame.tmp" : "/dev/shm/atropa_latest_frame.tmp";
                    const targetPath = isYouTube ? "/dev/shm/atropa_youtube_frame.jpg" : "/dev/shm/atropa_latest_frame.jpg";
                    fs.writeFileSync(tmpPath, jpegBuffer);
                    fs.renameSync(tmpPath, targetPath);
                    if (!isYouTube) {
                        sendWmqEvent("M:FRAME", jpegBuffer.length);
                    }
                } catch (writeErr) {}

                if (presenter && active && presenter.stdin.writable) {
                    const headerBuf = Buffer.alloc(20);
                    headerBuf.writeUInt32LE(jpegBuffer.length, 0);
                    headerBuf.writeInt32LE(videoBounds.x, 4);
                    headerBuf.writeInt32LE(videoBounds.y, 8);
                    headerBuf.writeInt32LE(videoBounds.width, 12);
                    headerBuf.writeInt32LE(videoBounds.height, 16);
                    let ok = presenter.stdin.write(headerBuf);
                    if (ok) {
                        ok = presenter.stdin.write(jpegBuffer);
                    }
                    if (!ok) {
                        await new Promise(resolve => presenter.stdin.once('drain', resolve));
                    }
                }
                const elapsed = Date.now() - startTime;
                const delay = Math.max(50 - elapsed, 0);
                await new Promise(resolve => setTimeout(resolve, delay));
            } catch (err) {
                await new Promise(resolve => setTimeout(resolve, 100));
            }
        }
    }

    page.on('load', async () => {
        if (!isYouTube) {
            await updateVideoBounds();
        }
        await startScreencast();
    });
    page.on('domcontentloaded', async () => {
        console.log("[PUPPETEER EVENT] DOMContentLoaded fired.");
        if (!isYouTube) {
            await updateVideoBounds();
        }
    });
    if (!isYouTube) {
        setInterval(async () => {
            await updateVideoBounds();
        }, 2000);
    }
    
    // Initial setup
    (async () => {
        if (!isYouTube) {
            await updateVideoBounds();
        }
        await startScreencast();
    })();

    if (url.includes("youtube.com")) {
        try {
            const consentButton = await page.$('button[aria-label*="Accept"], button[aria-label*="Agree"], ytd-button-renderer yt-button-shape button');
            if (consentButton) {
                console.log("[PUPPETEER] Clicking consent/agree button...");
                await consentButton.click();
                await new Promise(resolve => setTimeout(resolve, 2000));
            }
        } catch (e) {
            console.log("[PUPPETEER] Consent check skipped: " + e.message);
        }

        try {
            if (!url.includes("search_query=")) {
                console.log("[PUPPETEER] Identifying search input box...");
                const searchInputSelector = 'input[name="search_query"], input#search';
                await page.waitForSelector(searchInputSelector, { timeout: 10000 });
                const existingText = await page.$eval(searchInputSelector, el => el.value).catch(() => "");
                console.log(`[PUPPETEER] Existing search query text: "${existingText}"`);
                if (existingText.trim().toLowerCase() !== "atropa") {
                    console.log("[PUPPETEER] Clearing search input and typing 'Atropa'...");
                    await page.$eval(searchInputSelector, el => el.value = '');
                    await page.focus(searchInputSelector);
                    await page.type(searchInputSelector, "Atropa", { delay: 150 });
                } else {
                    console.log("[PUPPETEER] Search input already contains 'Atropa'. Skipping typing.");
                }
                console.log("[PUPPETEER] Performing search by direct navigation to filtered results...");
                // Directly navigate to the pre-filtered results page to load faster, bypassing filters dropdown
                const filteredSearchUrl = "https://www.youtube.com/results?search_query=Atropa&sp=EgIIAw%253D%253D";
                await page.goto(filteredSearchUrl, { waitUntil: "networkidle2" });
                console.log("[PUPPETEER] Jumped directly to filtered search results.");
                await new Promise(resolve => setTimeout(resolve, 5000));
            } else {
                console.log("[PUPPETEER] Custom query URL detected. Bypassing automatic Atropa search flow.");
            }
        } catch (e) {
            console.log("[PUPPETEER] Auncient search flow or direct navigation failed: " + e.message);
        }
    }

}

main().catch(err => {
    console.error("FATAL ERROR:", err);
});
