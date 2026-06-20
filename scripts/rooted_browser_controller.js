const puppeteer = require('puppeteer-core');
const { spawn } = require('child_process');
const { PNG } = require('pngjs');
const path = require('path');
const readline = require('readline');
const { ethers } = require('ethers');

let wmqContract = null;
let signer = null;

async function sendWmqEvent(cmd, args) {
    if (!wmqContract || !signer) return;
    try {
        const cmdHash = ethers.id(`${cmd}:${args}`);
        const txData = ethers.concat([
            "0x98d400c0", // writeDataPort
            ethers.zeroPadValue(cmdHash, 32)
        ]);
        const wmqAddress = await wmqContract.getAddress();
        await signer.sendTransaction({ to: wmqAddress, data: txData, gasLimit: 100000 });
    } catch (e) {
        // Suppress transaction errors if anvil is shut down dynamically
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
    const output = execSync(`solc --strict-assembly --evm-version shanghai "${absolutePath}" --bin`, { encoding: "utf8" });
    const lines = output.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    if (binIndex === -1) {
        throw new Error(`Could not find binary representation for ${yulPath}`);
    }
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    let url = process.argv[2] || "http://127.0.0.1:8000/zmachine.html";
    
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
        const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
        // Quick check to see if provider is online
        await provider.getNetwork();
        signer = await provider.getSigner(0);
        console.log(`[WMQ] Connected to local EVM provider. Signer: ${signer.address}`);
        
        const fs = require('fs');
        const yulPath = path.join(__dirname, "../solidity/bin/WinchesterMQ.yul");
        if (fs.existsSync(yulPath)) {
            const wmqBytecode = compileYul(yulPath);
            let wmqAddress = null;
            let factoryDeployed = false;

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

            // Write address to file for backend server reference
            try {
                fs.writeFileSync(path.join(__dirname, "../tmp/wmq_address.txt"), wmqAddress);
            } catch (err) {}

            // Set up a listener for LogPut event from WinchesterMQ (handshake mode)
            provider.on({
                address: wmqAddress,
                topics: ["0xa1bee1dae9af77dac73aa0459ed63b4d93fc6d29a1bee1dae9af77dac73aa045"]
            }, async (log) => {
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
                    const targetPrefix = isYouTube ? "YOUTUBE:" : "MAIN:";
                    if (commandStr && commandStr.startsWith(targetPrefix)) {
                        const actualCmd = commandStr.substring(targetPrefix.length);
                        console.log(`[WinchesterMQ Event Log] Routed input to ${isYouTube ? 'YouTube' : 'Main'}: ${actualCmd}`);
                        await handleInputCommand(actualCmd);
                    }
                } catch (err) {
                    console.error("[WMQ Listener ERR] Failed to parse input command block:", err);
                }
            });

            // Set up a listener for postEvent fast-path command logs
            provider.on({
                address: wmqAddress,
                topics: ["0xe1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1dae1da"]
            }, async (log) => {
                try {
                    const commandStr = Buffer.from(ethers.getBytes(log.data)).toString('utf8').replace(/\0/g, '').trim();
                    const targetPrefix = isYouTube ? "YOUTUBE:" : "MAIN:";
                    if (commandStr && commandStr.startsWith(targetPrefix)) {
                        const actualCmd = commandStr.substring(targetPrefix.length);
                        console.log(`[WinchesterMQ Fast Log] Routed input to ${isYouTube ? 'YouTube' : 'Main'}: ${actualCmd}`);
                        await handleInputCommand(actualCmd);
                    }
                } catch (err) {
                    console.error("[WMQ Fast Listener ERR] Failed to parse command:", err);
                }
            });
        }
    } catch (e) {
        console.log("[WMQ] WinchesterMQ not active or unreachable (Anvil offline). Using standard direct routing.");
    }

    // 1. Launch presenter process if not running in YouTube sub-browser frame-dumper mode
    const isYouTube = url.includes("youtube.com") || url.includes("youtube");
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
    let controlDown = false;
    let lastClickTime = 0;
    let clickCount = 1;
    let resizeTimeout = null;
    async function handleInputCommand(line) {
        if (line.includes("Streaming starting...")) {
            console.log(`[PRESENTER OUT] ${line}`);
            if (presenterReadyResolver) {
                presenterReadyResolver();
            }
            return;
        }
        const eventPrefix = isYouTube ? 'YOUTUBE:' : 'MAIN:';
        const parts = line.split(' ');
        const cmd = parts[0];
        try {
            if (cmd === 'MOUSE_MOVE') {
                const x = parseInt(parts[1]);
                const y = parseInt(parts[2]);
                await sendWmqEvent(eventPrefix + 'MOUSE_MOVE', `${x},${y}`);
                await page.mouse.move(x, y);
            } else if (cmd === 'MOUSE_DOWN') {
                const btn = parseInt(parts[1]);
                const button = linuxButtonMap[btn] || 'left';
                if (parts[2] !== undefined && parts[3] !== undefined) {
                    const x = parseInt(parts[2]);
                    const y = parseInt(parts[3]);
                    await page.mouse.move(x, y);
                }
                await sendWmqEvent(eventPrefix + 'MOUSE_DOWN', `${btn}`);
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
                if (parts[2] !== undefined && parts[3] !== undefined) {
                    const x = parseInt(parts[2]);
                    const y = parseInt(parts[3]);
                    await page.mouse.move(x, y);
                }
                await page.mouse.up({ button, clickCount });
            } else if (cmd === 'KEY_DOWN') {
                const key = parseInt(parts[1]); // Raw evdev keycode directly from Wayland client
                await sendWmqEvent(eventPrefix + 'KEY_DOWN', `${key}`);
                const keyName = linuxKeyMap[key];
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
            } else if (cmd === 'KEY_UP') {
                const key = parseInt(parts[1]); // Raw evdev keycode directly from Wayland client
                const keyName = linuxKeyMap[key];
                if (keyName === 'Control') {
                    controlDown = false;
                }
                if (keyName && keyName !== 'Enter') {
                    await page.keyboard.up(keyName);
                }
            } else if (cmd === 'MOUSE_SCROLL') {
                const axis = parseInt(parts[1]); // 0 for vertical scroll, 1 for horizontal
                const value = parseInt(parts[2]);
                await sendWmqEvent(eventPrefix + 'MOUSE_SCROLL', `${axis},${value}`);
                const deltaY = (axis === 0) ? value * 10 : 0;
                const deltaX = (axis === 1) ? value * 10 : 0;
                await page.mouse.wheel({ deltaX, deltaY });
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
                if (resizeTimeout) {
                    clearTimeout(resizeTimeout);
                }
                resizeTimeout = setTimeout(async () => {
                    console.log(`[PUPPETEER] Resizing viewport to ${width}x${height}`);
                    try {
                        await page.setViewport({ width, height });
                    } catch (viewportErr) {
                        // ignore viewport errors
                    }
                }, 20);
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
            "--disable-gpu",
            "--window-size=800,600",
            "--disable-dev-shm-usage",
            "--autoplay-policy=no-user-gesture-required",
            `--user-data-dir=${path.join(__dirname, "../tmp/puppeteer_chrome_profile_" + Date.now())}`
        ]
    });

    page = await browser.newPage();
    page.on('pageerror', err => {
        console.error('[PUPPETEER PAGE ERROR]', err.stack || err.message);
    });
    page.on('error', err => {
        console.error('[PUPPETEER PAGE CRASH]', err.message);
    });
    page.on('console', msg => {
        console.log('[PUPPETEER CONSOLE]', msg.text());
    });
    await page.setUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36 ROOTED Browser/1.0");
    await page.setViewport({ width: 800, height: 600 });

    let active = true;
    if (presenter) {
        presenter.on("exit", (code, signal) => {
            console.log(`[PRESENTER] Exited with code=${code}, signal=${signal}. Presenter window closed, keeping browser active for dashboard.`);
        });
    }

    // Screenshot streaming loop will be declared and run after page load to prevent early crashes.
    
    let lastReloadTime = 0;
    // Auto-unmute and auto-recover YouTube video player continuously in the background
    setInterval(async () => {
        try {
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

                // Check if playback error overlay is active
                const errScreen = document.querySelector('.ytp-error, .ytp-playability-error-supported-renderers, #error-screen');
                const hasError = !!(errScreen && errScreen.offsetWidth > 0 && errScreen.offsetHeight > 0);
                const errorText = document.querySelector('.ytp-error-message-text')?.innerText || "";

                const video = document.querySelector('video');
                if (!video) {
                    return { found: false, hasError, errorText };
                }

                return {
                    found: true,
                    currentTime: video.currentTime,
                    duration: video.duration,
                    paused: video.paused,
                    muted: video.muted,
                    title: document.title,
                    hasError,
                    errorText
                };
            });

            if (status.found) {
                const cur = (typeof status.currentTime === 'number') ? status.currentTime.toFixed(1) : '0';
                const dur = (typeof status.duration === 'number') ? status.duration.toFixed(1) : '0';
                console.log(`[VIDEO STATUS] Time: ${cur}s / ${dur}s, Paused: ${status.paused}, Muted: ${status.muted}, Error: ${status.hasError} (${status.errorText}), Title: "${status.title}"`);
                await sendWmqEvent("VIDEO_STATUS", `${cur},${dur},${status.paused},${status.muted}`);
            }

            if (status.hasError) {
                const now = Date.now();
                if (now - lastReloadTime > 15000) {
                    console.log("[PUPPETEER] YouTube error overlay detected. Reloading page...");
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

    let frameCount = 0;
    async function screenshotLoop() {
        while (active) {
            try {
                const startTime = Date.now();
                const jpegBuffer = await page.screenshot({ type: 'jpeg', quality: 75 });
                frameCount++;
                if (isYouTube || frameCount % 30 === 0) {
                    try {
                        const fs = require('fs');
                        const tmpPath = path.join(__dirname, "../frontend/latest_frame.tmp");
                        const targetPath = path.join(__dirname, "../frontend/latest_frame.jpg");
                        fs.writeFileSync(tmpPath, jpegBuffer);
                        fs.renameSync(tmpPath, targetPath);
                    } catch (writeErr) {
                        // ignore write errors
                    }
                }

                if (presenter && active && presenter.stdin.writable) {
                    const lenBuf = Buffer.alloc(4);
                    lenBuf.writeUInt32LE(jpegBuffer.length, 0);

                    let ok = presenter.stdin.write(lenBuf);
                    if (ok) {
                        ok = presenter.stdin.write(jpegBuffer);
                    }
                    if (!ok) {
                        await new Promise(resolve => presenter.stdin.once('drain', resolve));
                    }
                    if (frameCount % 90 === 0) {
                        console.log(`[STREAM] Sent ${frameCount} screenshot frames.`);
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
    screenshotLoop();

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
            console.log("[PUPPETEER] Identifying search input box...");
            const searchInputSelector = 'input[name="search_query"], input#search';
            await page.waitForSelector(searchInputSelector, { timeout: 10000 });
            console.log("[PUPPETEER] Search input identified. Typing 'Atropa'...");
            await page.focus(searchInputSelector);
            await page.type(searchInputSelector, "Atropa", { delay: 150 });
            console.log("[PUPPETEER] Performing search by direct navigation to filtered results...");
            // Directly navigate to the pre-filtered results page to load faster, bypassing filters dropdown
            const filteredSearchUrl = "https://www.youtube.com/results?search_query=Atropa&sp=EgIIAw%253D%253D";
            await page.goto(filteredSearchUrl, { waitUntil: "networkidle2" });
            console.log("[PUPPETEER] Jumped directly to filtered search results.");
            await new Promise(resolve => setTimeout(resolve, 5000));
        } catch (e) {
            console.log("[PUPPETEER] Auncient search flow or direct navigation failed: " + e.message);
        }
    }

}

main().catch(err => {
    console.error("FATAL ERROR:", err);
});
