const http = require("http");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");

const PORT = 3000;
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");

// Spawn the native ZMM VM MCP server process at startup
const mcpBinary = path.join(__dirname, "../tsfi2-deepseek/bin/tsfi_mcp_server");
const mcpCwd = path.join(__dirname, "../tsfi2-deepseek");

console.log(`[SERVER] Spawning native ZMM VM MCP server: ${mcpBinary}`);
const mcpProcess = spawn(mcpBinary, [], {
    cwd: mcpCwd,
    stdio: "inherit"
});

mcpProcess.on("error", err => {
    console.error("[SERVER] Failed to start native ZMM VM MCP server:", err);
});

// Spawn the resident VLM Synthesizer Python daemon at startup
const pythonDaemon = spawn("python3", ["-u", "scripts/render_vlm_synthesizer.py", "--daemon"], {
    cwd: path.join(__dirname, ".."),
    env: { ...process.env }
});

let daemonBuffer = "";
const currentListeners = {
    onLine: null,
    onComplete: null
};

pythonDaemon.stdout.on("data", data => {
    daemonBuffer += data.toString();
    const lines = daemonBuffer.split("\n");
    daemonBuffer = lines.pop(); // keep incomplete line
    
    for (const line of lines) {
        if (line.trim() === "__RENDER_COMPLETE__") {
            if (currentListeners.onComplete) {
                currentListeners.onComplete();
            }
        } else {
            if (currentListeners.onLine) {
                currentListeners.onLine(line + "\n");
            }
        }
    }
});

pythonDaemon.stderr.on("data", data => {
    console.error(`[PYTHON DAEMON STDERR] ${data.toString()}`);
});

pythonDaemon.on("error", err => {
    console.error("[SERVER] Failed to start Python Synthesizer Daemon:", err);
});

const renderQueue = [];
let renderInProgress = false;

function queueRender(payload, onLine, onComplete) {
    renderQueue.push({ payload, onLine, onComplete });
    processRenderQueue();
}

function processRenderQueue() {
    if (renderInProgress || renderQueue.length === 0) return;
    renderInProgress = true;
    
    const task = renderQueue[0];
    
    currentListeners.onLine = task.onLine;
    currentListeners.onComplete = () => {
        currentListeners.onLine = null;
        currentListeners.onComplete = null;
        task.onComplete();
        renderQueue.shift();
        renderInProgress = false;
        processRenderQueue();
    };
    
    pythonDaemon.stdin.write(JSON.stringify(task.payload) + "\n");
}

const https = require("https");
const MARKET_CACHE_PATH = path.join(__dirname, "../tmp/market_cache.json");
let marketFetchInProgress = false;
function fetchMarketData(addresses) {
    if (marketFetchInProgress || !addresses || addresses.length === 0) return;
    marketFetchInProgress = true;
    console.log(`[MARKET CACHE] Starting update using NoNukes reserves calculation for ${addresses.length} tokens...`);

    // 1. Fetch NoNukes price from DexScreener
    const url = "https://api.dexscreener.com/latest/dex/tokens/0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62";
    https.get(url, (res) => {
        let body = "";
        res.on("data", chunk => body += chunk);
        res.on("end", () => {
            let nonukesPriceUsd = 1.74; // default fallback
            try {
                const parsed = JSON.parse(body);
                if (parsed.pairs && parsed.pairs.length > 0) {
                    const pair = parsed.pairs.find(p => p.priceUsd) || parsed.pairs[0];
                    if (pair && pair.priceUsd) {
                        nonukesPriceUsd = parseFloat(pair.priceUsd);
                    }
                }
            } catch (e) {
                console.error("[MARKET CACHE] Failed to parse NoNukes price:", e.message);
            }
            console.log(`[MARKET CACHE] Current NoNukes USD price: ${nonukesPriceUsd}`);

            // 2. Load nonukes_pools.json
            let poolsMap = {};
            const poolsPath = path.join(__dirname, "../nonukes_pools.json");
            if (fs.existsSync(poolsPath)) {
                try {
                    poolsMap = JSON.parse(fs.readFileSync(poolsPath, "utf8"));
                } catch (e) {
                    console.error("[MARKET CACHE] Failed to read nonukes_pools.json:", e.message);
                }
            }

            // 3. Find reserves file path
            const os = require("os");
            const brainDir = path.join(os.homedir(), ".gemini/antigravity-cli/brain");
            let reservesPath = path.join(__dirname, "../nonukes_pulsex_reserves.json");
            if (fs.existsSync(brainDir)) {
                const subdirs = fs.readdirSync(brainDir);
                for (const subdir of subdirs) {
                    const checkPath = path.join(brainDir, subdir, "scratch/nonukes_pulsex_reserves.json");
                    if (fs.existsSync(checkPath)) {
                        reservesPath = checkPath;
                        break;
                    }
                }
            }

            // 4. Load reserves data
            let reservesMap = {};
            if (fs.existsSync(reservesPath)) {
                try {
                    reservesMap = JSON.parse(fs.readFileSync(reservesPath, "utf8"));
                } catch (e) {
                    console.error("[MARKET CACHE] Failed to read reserves file:", e.message);
                }
            } else {
                console.warn("[MARKET CACHE] Reserves file not found at:", reservesPath);
            }

            // 5. Compute price for each partner token
            const marketData = {};
            
            // Map partner token addresses to their LP pools
            const partnerToPool = {};
            for (const [poolAddr, poolInfo] of Object.entries(poolsMap)) {
                if (poolInfo.other_addr) {
                    partnerToPool[poolInfo.other_addr.toLowerCase()] = poolAddr.toLowerCase();
                }
            }

            addresses.forEach(addr => {
                const addrKey = addr.toLowerCase();
                const poolAddr = partnerToPool[addrKey];
                let priceUsd = "0";
                let liquidityUsd = 0;

                if (poolAddr && reservesMap[poolAddr]) {
                    const res = reservesMap[poolAddr];
                    const reserve0 = parseFloat(res.reserve0 || 0); // NoNukes
                    const reserve1 = parseFloat(res.reserve1 || 0); // Partner token
                    
                    if (reserve1 > 0) {
                        const priceInNoNukes = reserve0 / reserve1;
                        priceUsd = (priceInNoNukes * nonukesPriceUsd).toFixed(6);
                    }
                    liquidityUsd = reserve0 * nonukesPriceUsd * 2;
                }

                marketData[addrKey] = {
                    priceUsd: priceUsd,
                    priceChange24h: 0,
                    liquidityUsd: Math.round(liquidityUsd),
                    dexId: "pulsex",
                    quoteSymbol: "NONUKES"
                };
            });

            // 6. Write to market_cache.json
            try {
                fs.mkdirSync(path.dirname(MARKET_CACHE_PATH), { recursive: true });
                fs.writeFileSync(MARKET_CACHE_PATH, JSON.stringify({
                    timestamp: Date.now(),
                    data: marketData
                }, null, 2), "utf8");
                console.log(`[MARKET CACHE] Calculated and saved ${Object.keys(marketData).length} token price records based on NoNukes LP reserves.`);
            } catch (err) {
                console.error("[MARKET CACHE] Failed to write cache file:", err);
            }
            marketFetchInProgress = false;
        });
    }).on("error", (err) => {
        console.error("[MARKET CACHE] HTTP error fetching NoNukes price:", err.message);
        marketFetchInProgress = false;
    });
}



const MIME_TYPES = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "text/javascript",
    ".json": "application/json",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".gif": "image/gif",
    ".svg": "image/svg+xml"
};

const server = http.createServer((req, res) => {
    // Enable CORS for all requests
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");

    console.log(`[SERVER] ${req.method} ${req.url}`);
    if (req.method === "OPTIONS") {
        res.writeHead(204);
        res.end();
        return;
    }

    // API endpoint to serve config keys
    if (req.url === "/api/config") {
        if (fs.existsSync(CONFIG_PATH)) {
            const data = fs.readFileSync(CONFIG_PATH, "utf8");
            res.writeHead(200, { 
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Config file not found" }));
        }
        return;
    }

    // API endpoint to serve CABS Voxel Cards JSON database
    if (req.url === "/api/cabs-cards") {
        const cardsPath = path.join(__dirname, "../tsfi2-deepseek/assets/cabs_cards.json");
        if (fs.existsSync(cardsPath)) {
            const data = fs.readFileSync(cardsPath, "utf8");
            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Cards database not found" }));
        }
        return;
    }

    // API endpoint to serve NoNukes partner cards (aggregated)
    if (req.url === "/api/nonukes-cards") {
        const dataDir = path.join(__dirname, "../solidity/dysnomia/domain/data");
        const statusPath = path.join(__dirname, "../config/nonukes_qings_status.json");
        try {
            const cards = [];
            const addresses = [];
            let qingsMap = {};
            if (fs.existsSync(statusPath)) {
                try {
                    const statusData = JSON.parse(fs.readFileSync(statusPath, "utf8"));
                    statusData.forEach(item => {
                        qingsMap[item.address.toLowerCase()] = {
                            exists: item.exists,
                            qing: item.qing
                        };
                    });
                } catch (e) {
                    console.error("Failed to parse status path:", e);
                }
            }

            let marketCache = { timestamp: 0, data: {} };
            if (fs.existsSync(MARKET_CACHE_PATH)) {
                try {
                    marketCache = JSON.parse(fs.readFileSync(MARKET_CACHE_PATH, "utf8"));
                } catch (e) {
                    console.error("Failed to read market cache:", e);
                }
            }

            if (fs.existsSync(dataDir)) {
                const files = fs.readdirSync(dataDir);
                files.forEach(file => {
                    if (file.startsWith("0x") && file.endsWith(".json")) {
                        const content = fs.readFileSync(path.join(dataDir, file), "utf8");
                        const card = JSON.parse(content);
                        const addrKey = card.address.toLowerCase();
                        
                        addresses.push(card.address);

                        if (qingsMap[addrKey]) {
                            card.qingExists = qingsMap[addrKey].exists;
                            card.qingAddress = qingsMap[addrKey].qing;
                        } else {
                            card.qingExists = false;
                            card.qingAddress = null;
                        }

                        // Attach market metrics
                        if (marketCache.data[addrKey]) {
                            card.market = marketCache.data[addrKey];
                        } else {
                            card.market = {
                                priceUsd: "0",
                                priceChange24h: 0,
                                liquidityUsd: 0,
                                dexId: "unknown",
                                quoteSymbol: "WPLS"
                            };
                        }

                        cards.push(card);
                    }
                });
            }

            // Check cache age and trigger background update if expired (> 5 minutes)
            const cacheAge = Date.now() - marketCache.timestamp;
            if (cacheAge > 5 * 60 * 1000 && addresses.length > 0) {
                fetchMarketData(addresses);
            }

            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(JSON.stringify(cards));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
        }
        return;
    }

    // API endpoint to serve thunk storage reconciliation data
    if (req.url === "/api/thunk-storage") {
        const storagePath = path.join(__dirname, "../tsfi2-deepseek/evm_storage.json");
        if (fs.existsSync(storagePath)) {
            const data = fs.readFileSync(storagePath, "utf8");
            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(JSON.stringify({ storage: [] }));
        }
        return;
    }

    // POST API endpoint to save deployed keys into user_config.json
    if (req.url === "/api/save-keys" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const { address, keys, name, addresses } = payload;
                if (!address || !keys || !Array.isArray(keys)) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Invalid payload parameters" }));
                    return;
                }

                let configData = {};
                if (fs.existsSync(CONFIG_PATH)) {
                    configData = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
                }
                
                // Ensure key structure
                if (!configData.networks) configData.networks = {};
                if (!configData.networks.localhost) configData.networks.localhost = {};
                
                // Add deployment contract address
                configData.networks.localhost[name || `PKMinterToken_${address.substring(0, 6)}`] = address;

                // Save keys mapping relative to address
                if (!configData.saved_keys) configData.saved_keys = {};
                configData.saved_keys[address] = {
                    privateKeys: keys,
                    addresses: addresses || []
                };

                fs.writeFileSync(CONFIG_PATH, JSON.stringify(configData, null, 2), "utf8");
                res.writeHead(200, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ success: true }));
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }

    // API endpoint to serve the Markdown documentation
    if (req.url.startsWith("/api/docs")) {
        const urlObj = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const docName = urlObj.searchParams.get("doc") || "local_deployment_guide.md";
        const cleanDocName = path.basename(docName);
        const DOCS_PATH = path.join(__dirname, "../frontend", cleanDocName);
        if (fs.existsSync(DOCS_PATH)) {
            const data = fs.readFileSync(DOCS_PATH, "utf8");
            res.writeHead(200, { 
                "Content-Type": "text/markdown",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Docs not found" }));
        }
        return;
    }

    // API endpoint to list markdown/json files in the workspace
    if (req.url === "/api/workspace-files") {
        const baseDir = path.join(__dirname, "..");
        try {
            const getFiles = (dir) => {
                let results = [];
                const list = fs.readdirSync(dir);
                list.forEach(file => {
                    const fullPath = path.join(dir, file);
                    const stat = fs.statSync(fullPath);
                    if (stat && stat.isDirectory()) {
                        if (file !== "node_modules" && !file.startsWith(".")) {
                            results = results.concat(getFiles(fullPath));
                        }
                    } else if (file.endsWith(".md") || file.endsWith(".json")) {
                        results.push(path.relative(baseDir, fullPath));
                    }
                });
                return results;
            };
            const filesList = getFiles(baseDir);
            res.writeHead(200, { 
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(JSON.stringify(filesList));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
        }
        return;
    }

    // API endpoint to read a workspace file
    if (req.url.startsWith("/api/read-file")) {
        const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const relPath = parsedUrl.searchParams.get("path");
        const safePath = path.join(__dirname, "..", relPath);
        
        // Safety validation to prevent directory traversal
        if (!safePath.startsWith(path.join(__dirname, ".."))) {
            res.writeHead(403, { "Content-Type": "text/plain" });
            res.end("Forbidden");
            return;
        }

        if (fs.existsSync(safePath)) {
            const data = fs.readFileSync(safePath, "utf8");
            res.writeHead(200, { 
                "Content-Type": "text/plain",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "text/plain" });
            res.end("File not found");
        }
        return;
    }

    // API endpoint to write a workspace file
    if (req.url === "/api/write-file" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const { path: relPath, content } = payload;
                const safePath = path.join(__dirname, "..", relPath);
                
                // Safety validation to prevent directory traversal
                if (!safePath.startsWith(path.join(__dirname, ".."))) {
                    res.writeHead(403, { "Content-Type": "text/plain" });
                    res.end("Forbidden");
                    return;
                }

                fs.writeFileSync(safePath, content, "utf8");
                res.writeHead(200, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ success: true }));
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }
    // POST API endpoint to run a command or YULEXEC on the ZMM VM MCP server
    if (req.url === "/api/zmm-exec" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                let code = payload.code;
                if (!code && payload.name && payload.calldata) {
                    code = `YULEXEC "${payload.name}", "${payload.calldata}"`;
                }
                if (!code) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Missing 'code' or 'name' and 'calldata' parameter" }));
                    return;
                }

                const net = require("net");
                const socket = net.createConnection({ port: 10042 }, () => {
                    socket.write(JSON.stringify({
                        jsonrpc: "2.0",
                        method: "wave512.run",
                        params: { code: code },
                        id: 1
                    }));
                });

                let responseData = "";
                socket.on("data", chunk => {
                    responseData += chunk.toString();
                });

                socket.on("end", () => {
                    try {
                        const parsed = JSON.parse(responseData);
                        res.writeHead(200, { 
                            "Content-Type": "application/json",
                            "Access-Control-Allow-Origin": "*"
                        });
                        res.end(JSON.stringify(parsed));
                    } catch (e) {
                        res.writeHead(500, { "Content-Type": "application/json" });
                        res.end(JSON.stringify({ error: "Failed to parse ZMM VM response: " + e.message, raw: responseData }));
                    }
                });

                socket.on("error", err => {
                    res.writeHead(500, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "ZMM VM socket error: " + err.message }));
                });
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }
    // POST API endpoint to run dynamic speech synthesis via C controller
    if (req.url === "/api/synthesize-c" && req.method === "POST") {
        console.log("[SERVER] Incoming /api/synthesize-c POST request...");
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const { voice, phonemes, pitchContour } = payload;
                console.log(`[SERVER] Parameters: voice=${voice}, phonemes=${JSON.stringify(phonemes)}, pitchContourLength=${pitchContour ? pitchContour.length : 0}`);
                if (!voice || !Array.isArray(phonemes) || phonemes.length === 0) {
                    console.log("[SERVER] Error: Invalid payload parameters.");
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Invalid payload: 'voice' string and 'phonemes' array are required." }));
                    return;
                }

                // Call C speech generator binary with child_process
                const { execFile } = require("child_process");
                const binaryPath = path.join(__dirname, "../tsfi2-deepseek/tests/test_speech_c_controller");
                const cwd = path.join(__dirname, "../tsfi2-deepseek");

                const env = { ...process.env };
                if (Array.isArray(pitchContour)) {
                    env.PITCH_CONTOUR = pitchContour.join(",");
                }

                console.log(`[SERVER] Spawning binary: ${binaryPath} with env PITCH_CONTOUR=${env.PITCH_CONTOUR}`);
                execFile(binaryPath, [voice, ...phonemes], { cwd, env }, (error, stdout, stderr) => {
                    if (error) {
                        console.error("[SERVER] execFile error:", error.message);
                        console.error("[SERVER] execFile stdout:", stdout);
                        console.error("[SERVER] execFile stderr:", stderr);
                        res.writeHead(500, { "Content-Type": "application/json" });
                        res.end(JSON.stringify({ error: error.message, stderr }));
                        return;
                    }

                    console.log("[SERVER] execFile completed successfully. Stdout length:", stdout.length);
                    // The C controller outputs 'karateka_c_synthesis.wav' inside the cwd
                    const wavPath = path.join(cwd, "karateka_c_synthesis.wav");
                    if (fs.existsSync(wavPath)) {
                        console.log("[SERVER] Generated WAV file found. Reading and sending...");
                        const wavBuffer = fs.readFileSync(wavPath);
                        res.writeHead(200, {
                            "Content-Type": "audio/wav",
                            "Access-Control-Allow-Origin": "*"
                        });
                        res.end(wavBuffer);
                    } else {
                    }
                });
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }

    // POST API endpoint to render specific Dragon DNA frame
    if (req.url === "/api/render-dna" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const frameIdx = Number(payload.frame) || 50;
                console.log(`[SERVER] Rendering DNA frame ${frameIdx}...`);

                // If customized slider values are provided, pack and write them to dragon.dna first
                if (payload.g_x !== undefined) {
                    const dnaPath = path.join(__dirname, "../tsfi2-deepseek/assets/dragon.dna");
                    if (fs.existsSync(dnaPath)) {
                        const hexToRgb = (hex) => {
                            const bigint = parseInt(hex.replace("#", ""), 16);
                            return [(bigint >> 16) & 255, (bigint >> 8) & 255, bigint & 255];
                        };

                        const [r, g, b] = hexToRgb(payload.scaleColor || "#ff0000");
                        const [er, eg, eb] = hexToRgb(payload.eyeColor || "#ffff00");

                        const buf = Buffer.alloc(31);
                        buf.writeFloatLE(payload.g_x, 0);
                        buf.writeFloatLE(payload.g_y, 4);
                        buf.writeFloatLE(payload.stretch, 8);
                        buf.writeFloatLE(payload.pulse, 12);
                        buf.writeFloatLE(payload.fire, 16);
                        buf.writeFloatLE(payload.light, 20);
                        buf.writeUInt8(r, 24);
                        buf.writeUInt8(g, 25);
                        buf.writeUInt8(b, 26);
                        buf.writeUInt8(er, 27);
                        buf.writeUInt8(eg, 28);
                        buf.writeUInt8(eb, 29);
                        buf.writeUInt8(Number(payload.eyeCount) || 2, 30);

                        try {
                            const fd = fs.openSync(dnaPath, "r+");
                            fs.writeSync(fd, buf, 0, 31, 8 + frameIdx * 31);
                            fs.closeSync(fd);
                            console.log(`[SERVER] Updated dragon.dna at frame ${frameIdx} with custom slider parameters.`);
                        } catch (writeErr) {
                            console.error("[SERVER] Failed to write custom DNA parameters:", writeErr);
                        }
                    }
                }

                const { spawn } = require("child_process");
                const steps = payload.steps !== undefined ? Number(payload.steps) : 4;
                const cfg = payload.cfg !== undefined ? Number(payload.cfg) : 1.5;
                const promptOverride = payload.promptOverride ? payload.promptOverride.replace(/"/g, '\\"') : "";
                
                const args = [frameIdx.toString()];
                if (steps) args.push("--steps", steps.toString());
                if (cfg) args.push("--cfg", cfg.toString());
                if (promptOverride) args.push("--prompt", promptOverride);
                if (payload.address) args.push("--address", payload.address);
                if (payload.hypobar) args.push("--hypobar", payload.hypobar.toString());
                if (payload.epibar) args.push("--epibar", payload.epibar.toString());

                console.log(`[SERVER] Dispatching rendering request to daemon: steps=${steps}, cfg=${cfg}, prompt=${payload.promptOverride || ""}`);
                
                res.writeHead(200, {
                    "Content-Type": "text/event-stream",
                    "Cache-Control": "no-cache",
                    "Connection": "keep-alive",
                    "Access-Control-Allow-Origin": "*"
                });

                let fullLog = "";
                queueRender(
                    {
                        frame: frameIdx,
                        steps,
                        cfg,
                        promptOverride: payload.promptOverride,
                        address: payload.address,
                        hypobar: payload.hypobar || 0,
                        epibar: payload.epibar || 0
                    },
                    (line) => {
                        fullLog += line;
                        res.write(`data: ${JSON.stringify({ type: "stdout", content: line })}\n\n`);
                    },
                    () => {
                        console.log("[SERVER] VLM Synthesizer DNA frame rendered successfully via daemon.");
                        res.write(`data: ${JSON.stringify({ type: "success", url: payload.address ? `assets/${payload.address.toLowerCase()}.png` : "assets/storybook/page_dragon_dna.png", logs: fullLog })}\n\n`);
                        res.end();
                    }
                );
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }


    // Serve static files from frontend directory
    const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    let filePath = parsedUrl.pathname === "/" ? "/index.html" : parsedUrl.pathname;
    if (filePath.startsWith("/frontend/")) {
        filePath = filePath.replace("/frontend/", "/");
    }
    let absolutePath = path.join(__dirname, "../frontend", filePath);
    
    // Safety check to prevent directory traversal
    if (parsedUrl.pathname.startsWith("/Wallet/bin/Contracts/")) {
        // Expose Solidity compiled output files securely
        absolutePath = path.join(__dirname, "..", filePath);
    } else if (parsedUrl.pathname.startsWith("/node_modules/")) {
        // Expose local node_modules packages (e.g. ethers) securely
        absolutePath = path.join(__dirname, "..", filePath);
    } else if (parsedUrl.pathname.startsWith("/assets/")) {
        // Expose assets directory (e.g. DNA rendered images) securely
        absolutePath = path.join(__dirname, "..", filePath);
    } else if (parsedUrl.pathname === "/config/user_config.json") {
        absolutePath = CONFIG_PATH;
    } else if (!absolutePath.startsWith(path.join(__dirname, "../frontend"))) {
        res.writeHead(403, { "Content-Type": "text/plain" });
        res.end("Forbidden");
        return;
    }

    const serveFile = (fileToServe) => {
        const ext = path.extname(fileToServe);
        const contentType = MIME_TYPES[ext] || "application/octet-stream";
        res.writeHead(200, { "Content-Type": contentType });
        fs.createReadStream(fileToServe).pipe(res);
    };

    fs.exists(absolutePath, (exists) => {
        if (exists && fs.statSync(absolutePath).isFile()) {
            serveFile(absolutePath);
        } else {
            const ext = path.extname(absolutePath);
            if (!ext) {
                const htmlPath = absolutePath + ".html";
                fs.exists(htmlPath, (htmlExists) => {
                    if (htmlExists && fs.statSync(htmlPath).isFile()) {
                        serveFile(htmlPath);
                    } else {
                        res.writeHead(404, { "Content-Type": "text/plain" });
                        res.end("404 Not Found");
                    }
                });
            } else {
                res.writeHead(404, { "Content-Type": "text/plain" });
                res.end("404 Not Found");
            }
        }
    });
});

server.listen(PORT, "127.0.0.1", () => {
    console.log(`==================================================`);
    console.log(`DApp Dashboard Server running at http://127.0.0.1:${PORT}`);
    console.log(`==================================================`);
});
