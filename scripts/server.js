const http = require("http");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");

const PORT = 3000;
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");
let dilemmaLog = [];

const AlsaSynth = require("./alsa_synth");
const { loreAnalysisCache, performLoreAnalysis, processAnnotationReplies } = require("./lore_analyzer");
const { spider } = require("./spider_lore_tags");
const { buildGlossary } = require("./build_lore_glossary");
const marketCache = require("./market_cache");
const { fetchMarketData, MARKET_CACHE_PATH } = marketCache;

global.alsaSynthInstance = new AlsaSynth();

// Spawn the native ZMM VM MCP server process at startup
const mcpBinary = path.join(__dirname, "../tsfi2-deepseek/bin/tsfi_mcp_server");
const mcpCwd = path.join(__dirname, "../tsfi2-deepseek");

// Load stored private keys from config for VM startup
let defaultPkiKey = "";
try {
    if (fs.existsSync(CONFIG_PATH)) {
        const configData = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
        if (configData.default && configData.default.keys && configData.default.keys.length > 0) {
            defaultPkiKey = configData.default.keys[0];
        } else if (configData.saved_keys) {
            const keys = Object.values(configData.saved_keys)[0];
            if (Array.isArray(keys) && keys.length > 0) {
                defaultPkiKey = keys[0];
            } else if (keys && Array.isArray(keys.privateKeys) && keys.privateKeys.length > 0) {
                defaultPkiKey = keys.privateKeys[0];
            }
        }
    }
} catch (e) {
    console.error("[SERVER] Failed to load default PKI key from config:", e.message);
}

// Reset ZMM VM storage context for a fresh newly-generated execution run
const evmStoragePath = path.join(__dirname, "../tsfi2-deepseek/evm_storage.json");
const rootStoragePath = path.join(__dirname, "../evm_storage.json");
try {
    const emptyStorage = JSON.stringify({ storage: [] }, null, 2);
    fs.writeFileSync(evmStoragePath, emptyStorage, "utf8");
    fs.writeFileSync(rootStoragePath, emptyStorage, "utf8");
    console.log("[SERVER] Storage cleared on startup to allow clean contract initialization.");
} catch (err) {
    console.error("[SERVER] Failed to clean ZMM VM storage state:", err.message);
}

console.log(`[SERVER] Spawning native ZMM VM MCP server: ${mcpBinary}`);
const mcpProcess = spawn(mcpBinary, [], {
    cwd: mcpCwd,
    stdio: ["pipe", "pipe", "inherit"],
    env: {
        ...process.env,
        TSFI_DEFAULT_PKI_KEY: defaultPkiKey
    }
});

const pendingRequests = new Map();
const readline = require("readline");
const mcpInterface = readline.createInterface({
    input: mcpProcess.stdout,
    terminal: false
});

mcpInterface.on("line", line => {
    const trimmed = line.trim();
    if (trimmed.startsWith("{")) {
        try {
            const response = JSON.parse(trimmed);
            if (response && response.id !== undefined) {
                const cb = pendingRequests.get(response.id);
                if (cb) {
                    pendingRequests.delete(response.id);
                    cb(null, response);
                }
            }
        } catch (e) {
            console.error("[SERVER] Failed to parse JSON line from ZMM VM MCP stdout:", line, e);
        }
    } else if (trimmed) {
        console.log("[ZMM VM MCP STDOUT]", trimmed);
    }
});

mcpProcess.on("error", err => {
    console.error("[SERVER] Failed to start native ZMM VM MCP server:", err);
});

const initializedContracts = new Set();
function runZmmCommand(code) {
    return new Promise((resolve, reject) => {
        const id = Math.floor(Math.random() * 1000000);
        const rpcPayload = {
            jsonrpc: "2.0",
            method: "wave512.run",
            params: { code: code },
            id: id
        };
        const timeoutId = setTimeout(() => {
            if (pendingRequests.has(id)) {
                pendingRequests.delete(id);
                reject(new Error("ZMM VM execution timed out"));
            }
        }, 5000);
        pendingRequests.set(id, (err, response) => {
            clearTimeout(timeoutId);
            if (err) reject(err);
            else resolve(response);
        });
        mcpProcess.stdin.write(JSON.stringify(rpcPayload) + "\n");
    });
}

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

try {
    performLoreAnalysis();
} catch (e) {
    console.error("[SERVER] Failed to run initial lore analysis:", e);
}

const server = http.createServer(async (req, res) => {
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

    // Proxy NoNukes dashboard requests to python server on port 8080
    if (
        req.url.startsWith("/api/nonukes/") || 
        req.url.startsWith("/api/data") || 
        req.url.startsWith("/api/pools") || 
        req.url.startsWith("/api/ignore")
    ) {
        const proxyReq = http.request({
            host: "127.0.0.1",
            port: 8080,
            path: req.url,
            method: req.method,
            headers: req.headers
        }, (proxyRes) => {
            res.writeHead(proxyRes.statusCode, proxyRes.headers);
            proxyRes.pipe(res);
        });
        proxyReq.on("error", (err) => {
            res.writeHead(502, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Failed to connect to dashboard backend", details: err.message }));
        });
        req.pipe(proxyReq);
        return;
    }

    // Server-Side Event Feed for Headless AI performance broadcasting
    if (req.url.startsWith("/api/synth-feed") && req.method === "GET") {
        const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const sessionId = parsedUrl.searchParams.get("sessionId") || "global";

        req.socket.setNoDelay(true);

        res.writeHead(200, {
            "Content-Type": "text/event-stream",
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
            "Access-Control-Allow-Origin": "*",
            "X-Accel-Buffering": "no"
        });
        res.flushHeaders();
        
        if (!global.synthSessions) {
            global.synthSessions = {};
        }
        if (!global.synthSessions[sessionId]) {
            global.synthSessions[sessionId] = [];
        }
        global.synthSessions[sessionId].push(res);

        // Send connection initialization comment to finalize HTTP handshake
        res.write(": ok\n\n");
        
        req.on("close", () => {
            if (global.synthSessions[sessionId]) {
                global.synthSessions[sessionId] = global.synthSessions[sessionId].filter(c => c !== res);
                if (global.synthSessions[sessionId].length === 0) {
                    delete global.synthSessions[sessionId];
                }
            }
        });
        return;
    }

    if (req.url.startsWith("/api/synth-feed") && req.method === "POST") {
        const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const sessionId = parsedUrl.searchParams.get("sessionId") || "global";

        let body = "";
        req.on("data", chunk => body += chunk.toString());
        req.on("end", () => {
             try {
                  const data = JSON.parse(body);
                  
                  // Route to CPU-level ALSA synthesizer
                  if (data.type === 'synth_poke') {
                      const freq = parseFloat(data.freq);
                      const control = parseInt(data.control);
                      const gateOn = control & 1;
                      if (global.alsaSynthInstance && !isNaN(freq)) {
                          if (gateOn) {
                              global.alsaSynthInstance.noteOn(freq);
                          } else {
                              global.alsaSynthInstance.noteOff(freq);
                          }
                      }
                  } else if (data.type === 'synth_reset') {
                      if (global.alsaSynthInstance) {
                          global.alsaSynthInstance.reset();
                      }
                  }

                  let clientCount = 0;
                  if (global.synthSessions && global.synthSessions[sessionId]) {
                      clientCount = global.synthSessions[sessionId].length;
                      global.synthSessions[sessionId].forEach(client => {
                          client.write(`data: ${JSON.stringify(data)}\n\n`);
                      });
                  }
                  console.log(`[SERVER] POST /api/synth-feed?sessionId=${sessionId} - Broadcasted to ${clientCount} clients`);
                  res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
                  res.end(JSON.stringify({ success: true }));
            } catch (err) {
                console.error(`[SERVER] Error in POST /api/synth-feed: ${err.message}`);
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
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

    // API endpoint to serve current NoNukes price in USD
    if (req.url === "/api/nonukes-price") {
        res.writeHead(200, {
            "Content-Type": "application/json",
            "Access-Control-Allow-Origin": "*"
        });
        res.end(JSON.stringify({ priceUsd: marketCache.currentNoNukesPriceUsd }));
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

            let marketCacheData = { timestamp: 0, data: {} };
            if (fs.existsSync(MARKET_CACHE_PATH)) {
                try {
                    marketCacheData = JSON.parse(fs.readFileSync(MARKET_CACHE_PATH, "utf8"));
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
                        if (marketCacheData.data[addrKey]) {
                            card.market = marketCacheData.data[addrKey];
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
            const cacheAge = Date.now() - marketCacheData.timestamp;
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

    // API endpoint to list all loaded ZMM Yul objects/contracts
    if (req.url === "/api/zmm-objects") {
        const cacheDir = path.join(__dirname, "../tsfi2-deepseek/.yul_cache");
        try {
            if (fs.existsSync(cacheDir)) {
                const files = fs.readdirSync(cacheDir);
                const objects = files
                    .filter(f => f.endsWith(".hex"))
                    .map(f => {
                        const stat = fs.statSync(path.join(cacheDir, f));
                        return {
                            name: f.slice(0, -4),
                            sizeBytes: stat.size,
                            modified: Math.round(stat.mtimeMs)
                        };
                    });
                res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
                res.end(JSON.stringify(objects));
            } else {
                res.writeHead(404, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: "Cache directory not found" }));
            }
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
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

    // API endpoint to list lore documentation files
    if (req.url.startsWith("/api/lore/list")) {
        const urlObj = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const scope = urlObj.searchParams.get("scope") || "local";
        
        let targetDirs = [];
        if (scope === "brain") {
            const homeDir = process.env.HOME || "/home/mariarahel";
            const brainRoot = path.join(homeDir, ".gemini/antigravity-cli/brain");
            if (fs.existsSync(brainRoot)) {
                // Find all .md files under brain/ recursively using a helper function
                const getMarkdownFiles = (dir) => {
                    let results = [];
                    if (!fs.existsSync(dir)) return results;
                    const list = fs.readdirSync(dir);
                    list.forEach(file => {
                        const fullPath = path.join(dir, file);
                        let stat;
                        try {
                            stat = fs.statSync(fullPath);
                        } catch (e) {
                            return; // skip if cannot stat
                        }
                        if (stat && stat.isDirectory()) {
                            results = results.concat(getMarkdownFiles(fullPath));
                        } else if (file.endsWith(".md") && !file.includes(".system_generated")) {
                            results.push({ fullPath, relativePath: path.relative(brainRoot, fullPath) });
                        }
                    });
                    return results;
                };
                targetDirs = getMarkdownFiles(brainRoot);
            }
        } else {
            const loreDir = path.join(__dirname, "../lore");
            if (fs.existsSync(loreDir)) {
                const files = fs.readdirSync(loreDir);
                targetDirs = files
                    .filter(file => file.endsWith(".md") || file.endsWith(".lore"))
                    .map(file => ({
                        fullPath: path.join(loreDir, file),
                        relativePath: file
                    }));
            }
        }

        try {
            // Execute git status to check for untracked or modified files under lore/ (only for local scope)
            let gitStatusMap = {};
            if (scope === "local") {
                try {
                    const { execSync } = require("child_process");
                    const gitOut = execSync("git status --porcelain lore", { encoding: "utf8" });
                    gitOut.split("\n").forEach(line => {
                        if (line.trim()) {
                            const code = line.substring(0, 2);
                            const filePath = line.substring(3).trim();
                            const fileName = path.basename(filePath);
                            if (code.includes("??")) {
                                gitStatusMap[fileName] = "untracked";
                            } else {
                                gitStatusMap[fileName] = "modified";
                            }
                        }
                    });
                } catch (gitErr) {
                    // Fail silently if not a git repository
                }
            }

            const results = targetDirs.map(item => {
                const fullPath = item.fullPath;
                const file = item.relativePath;
                const stat = fs.statSync(fullPath);
                let isReviewed = false;
                try {
                    if (stat.size > 0) {
                        const fd = fs.openSync(fullPath, 'r');
                        const buffer = Buffer.alloc(200);
                        const position = Math.max(0, stat.size - 200);
                        const bytesRead = fs.readSync(fd, buffer, 0, 200, position);
                        const lastBytes = buffer.toString('utf8', 0, bytesRead);
                        fs.closeSync(fd);
                        if (lastBytes.includes('AUNCIENT_STATUS: REVIEWED')) {
                            isReviewed = true;
                        }
                    }
                } catch (readErr) {
                    // Ignore read errors
                }

                return {
                    name: file,
                    sizeBytes: stat.size,
                    modified: Math.round(stat.mtimeMs),
                    gitStatus: scope === "local" ? (gitStatusMap[file] || null) : null,
                    reviewed: isReviewed,
                    louScore: (loreAnalysisCache[file] ? loreAnalysisCache[file].score : 0),
                    louReasons: (loreAnalysisCache[file] ? loreAnalysisCache[file].reasons : []),
                    scope: scope
                };
            });

            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(JSON.stringify(results));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
        }
        return;
    }

    // API endpoint to read a specific lore file content
    if (req.url.startsWith("/api/lore/content") && req.method === "GET") {
        const urlObj = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
        const fileName = urlObj.searchParams.get("file");
        const scope = urlObj.searchParams.get("scope") || "local";
        if (!fileName) {
            res.writeHead(400, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "File parameter missing" }));
            return;
        }

        let filePath;
        if (scope === "brain") {
            const homeDir = process.env.HOME || "/home/mariarahel";
            // Ensure no path traversal outside brain/
            const cleanRel = fileName.replace(/\.\./g, "");
            filePath = path.join(homeDir, ".gemini/antigravity-cli/brain", cleanRel);
        } else {
            const cleanName = path.basename(fileName);
            filePath = path.join(__dirname, "../lore", cleanName);
        }

        if (fs.existsSync(filePath)) {
            const data = fs.readFileSync(filePath, "utf8");
            res.writeHead(200, {
                "Content-Type": "text/plain; charset=utf-8",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(data);
        } else {
            res.writeHead(404, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "Lore file not found" }));
        }
        return;
    }

    // API endpoint to write/save a lore file content
    if (req.url.startsWith("/api/lore/save") && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const { file, content, scope } = payload;
                const fileScope = scope || "local";
                if (!file || content === undefined) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Invalid payload parameters" }));
                    return;
                }

                let filePath;
                if (fileScope === "brain") {
                    const homeDir = process.env.HOME || "/home/mariarahel";
                    const cleanRel = file.replace(/\.\./g, "");
                    filePath = path.join(homeDir, ".gemini/antigravity-cli/brain", cleanRel);
                } else {
                    const cleanName = path.basename(file);
                    filePath = path.join(__dirname, "../lore", cleanName);
                }

                // Process automated replies to annotations
                const processedContent = processAnnotationReplies(content, path.basename(file));
                
                fs.writeFileSync(filePath, processedContent, "utf8");
                if (fileScope === "local") {
                    try {
                        performLoreAnalysis();
                    } catch (e) {}

                    // Auto-commit saved lore file safely without staging any binaries
                    try {
                        const { execSync } = require("child_process");
                        execSync(`git add lore/${path.basename(file)}`, { cwd: path.join(__dirname, "..") });
                        execSync(`git commit -m "Auto-commit lore: ${path.basename(file)}"`, { cwd: path.join(__dirname, "..") });
                        console.log(`[SERVER] Auto-committed lore/${path.basename(file)}`);
                    } catch (gitErr) {
                        console.error("[SERVER] Failed to auto-commit lore file:", gitErr.message);
                    }
                }

                res.writeHead(200, {
                    "Content-Type": "application/json",
                    "Access-Control-Allow-Origin": "*"
                });
                res.end(JSON.stringify({ success: true, content: processedContent }));
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

    if (req.url === "/api/vanity-keys" && req.method === "GET") {
        try {
            const homeDir = process.env.HOME || "/home/mariarahel";
            const files = [
                path.join(__dirname, "../found_addresses.txt"),
                path.join(homeDir, "repkeys.txt"),
                path.join(homeDir, "repkeys2.txt")
            ];
            const matches = [];
            const seen = new Set();
            for (const file of files) {
                if (!fs.existsSync(file)) continue;
                const data = fs.readFileSync(file, "utf8");
                const blocks = data.split("==================================================");
                for (const block of blocks) {
                    if (!block.includes("MATCH FOUND!")) continue;
                    const addrMatch = block.match(/Address:\s+(0x[a-fA-F0-9]+)/);
                    const pkMatch = block.match(/Final Private Key:\s+(0x[a-fA-F0-9]+)/);
                    if (addrMatch && pkMatch) {
                        const address = addrMatch[1];
                        const privateKey = pkMatch[1];
                        if (seen.has(address.toLowerCase())) continue;
                        seen.add(address.toLowerCase());
                        const onesCount = (address.toLowerCase().substring(2).match(/1/g) || []).length;
                        matches.push({ address, privateKey, ones: onesCount });
                    }
                }
            }
            res.writeHead(200, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ keys: matches }));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
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
    // GET/POST API endpoints for dilemma logging
    if (req.url === "/api/dilemma-log" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => body += chunk.toString());
        req.on("end", () => {
            try {
                const data = JSON.parse(body);
                dilemmaLog.push({
                    timestamp: new Date().toISOString(),
                    event: data.event,
                    source: data.source,
                    details: data.details || ""
                });
                if (dilemmaLog.length > 500) dilemmaLog.shift();

                // Automatically capture YouTube telemetry if logged
                let isYoutube = false;
                let videoId = "";
                let title = "";
                
                if (data.event && data.event.startsWith("YOUTUBE:")) {
                    isYoutube = true;
                    videoId = data.event.substring(8).trim();
                    title = data.details || "Unknown YouTube Video";
                } else if (data.details && (data.details.includes("youtube.com/watch") || data.details.includes("youtu.be/"))) {
                    isYoutube = true;
                    const url = data.details;
                    const match = url.match(/(?:v=|\/)([\w-]{11})(?:\?|&|$)/);
                    videoId = match ? match[1] : "unknown";
                    title = data.event || "YouTube Playback";
                }
                
                if (isYoutube && videoId) {
                    const recordPath = path.join(__dirname, "../tmp/youtube_telemetry.json");
                    let records = [];
                    if (fs.existsSync(recordPath)) {
                        try {
                            records = JSON.parse(fs.readFileSync(recordPath, "utf8"));
                        } catch (e) {
                            records = [];
                        }
                    }
                    records.push({
                        timestamp: Date.now(),
                        videoId,
                        title
                    });
                    fs.mkdirSync(path.dirname(recordPath), { recursive: true });
                    fs.writeFileSync(recordPath, JSON.stringify(records, null, 2), "utf8");
                    console.log(`[SERVER] Telemetry logged: YouTube video "${title}" (v=${videoId})`);
                }

                res.writeHead(200, { 
                    "Content-Type": "application/json", 
                    "Access-Control-Allow-Origin": "*" 
                });
                res.end(JSON.stringify({ status: "ok" }));
            } catch (e) {
                res.writeHead(400, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: e.message }));
            }
        });
        return;
    }
    if (req.url === "/api/dilemma-log" && req.method === "GET") {
        res.writeHead(200, { 
            "Content-Type": "application/json", 
            "Access-Control-Allow-Origin": "*" 
        });
        res.end(JSON.stringify(dilemmaLog));
        return;
    }
    if (req.url === "/api/hypervisor-status" && req.method === "GET") {
        const net = require("net");
        const exec = require("child_process").exec;
        
        const checkMcp = new Promise((resolve) => {
            const socket = net.createConnection({ port: 10042 }, () => {
                socket.end();
                resolve(true);
            });
            socket.on("error", () => {
                resolve(false);
            });
        });

        const checkScan = new Promise((resolve) => {
            exec("pgrep -f monitor_pulsex.py", (err, stdout) => {
                if (err || !stdout.trim()) {
                    resolve(false);
                } else {
                    resolve(true);
                }
            });
        });

        Promise.all([checkMcp, checkScan]).then(([mcpOnline, scanOnline]) => {
            res.writeHead(200, {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*"
            });
            res.end(JSON.stringify({
                status: "ok",
                services: {
                    dapp_server: true,
                    zmm_mcp_server: mcpOnline,
                    pulsex_scan_daemon: scanOnline
                }
            }));
        });
        return;
    }

    // POST API endpoint to run a command or YULEXEC on the ZMM VM MCP server
    if (req.url === "/api/zmm-exec" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", async () => {
            try {
                const payload = JSON.parse(body);
                
                let nameToInit = payload.name;
                if (!nameToInit && payload.code) {
                    const m = payload.code.match(/YULEXEC\s+"(dynamic_[0-9a-fA-F]+)"/);
                    if (m) nameToInit = m[1];
                }
                if (nameToInit && nameToInit.startsWith("dynamic_")) {
                    const name = nameToInit;
                    if (!initializedContracts.has(name)) {
                        try {
                            const addrHex = name.replace("dynamic_", "");
                            const addrBigInt = BigInt("0x" + addrHex);
                            console.log(`[SERVER] Auto-initializing dynamic contract: ${name} at addr ${addrBigInt}`);
                            await runZmmCommand(`YULINIT "${name}", "dynamic", ${addrBigInt.toString()}`);
                            initializedContracts.add(name);
                        } catch (err) {
                            console.error(`[SERVER] Failed to auto-initialize dynamic contract ${name}:`, err.message);
                        }
                    }
                }

                let rpcPayload = null;

                if (payload.jsonrpc && payload.method) {
                    rpcPayload = payload;
                } else {
                    let code = payload.code;
                    if (!code && payload.name && payload.calldata) {
                        code = `YULEXEC "${payload.name}", "${payload.calldata}"`;
                    }
                    if (!code) {
                        res.writeHead(400, { "Content-Type": "application/json" });
                        res.end(JSON.stringify({ error: "Missing 'code' or 'name' and 'calldata' parameter or 'jsonrpc' payload" }));
                        return;
                    }

                    if (code.startsWith("wave512.dilemma_log")) {
                        const jsonPart = code.substring("wave512.dilemma_log".length).trim();
                        let params = {};
                        if (jsonPart) {
                            try {
                                params = JSON.parse(jsonPart);
                            } catch (e) {
                                console.error("[SERVER] Failed to parse wave512.dilemma_log JSON part:", e);
                            }
                        }
                        rpcPayload = {
                            jsonrpc: "2.0",
                            method: "wave512.dilemma_log",
                            params: params,
                            id: 1
                        };
                    } else {
                        rpcPayload = {
                            jsonrpc: "2.0",
                            method: "wave512.run",
                            params: { code: code },
                            id: 1
                        };
                    }
                }

                if (rpcPayload.id === undefined || rpcPayload.id === null) {
                    rpcPayload.id = Math.floor(Math.random() * 1000000);
                }

                const reqId = rpcPayload.id;
                const timeoutId = setTimeout(() => {
                    if (pendingRequests.has(reqId)) {
                        pendingRequests.delete(reqId);
                        res.writeHead(504, { "Content-Type": "application/json" });
                        res.end(JSON.stringify({ error: "ZMM VM execution timed out" }));
                    }
                }, 5000);

                pendingRequests.set(reqId, (err, response) => {
                    clearTimeout(timeoutId);
                    if (err) {
                        res.writeHead(500, { "Content-Type": "application/json" });
                        res.end(JSON.stringify({ error: err.message }));
                    } else {
                        // Check if the ZMM VM execution target triggered diyat actions
                        try {
                            if (response && response.result && response.result.logs) {
                                const logsStr = JSON.stringify(response.result.logs);
                                if (logsStr.includes("TaxPaid") || logsStr.includes("diyat")) {
                                    console.log(`[SERVER] [Diyat Audit] Active dynamic Diyat tax fee payment detected in response logs.`);
                                }
                            }
                        } catch (e) {}

                        res.writeHead(200, { 
                            "Content-Type": "application/json",
                            "Access-Control-Allow-Origin": "*"
                        });
                        res.end(JSON.stringify(response));
                    }
                });

                mcpProcess.stdin.write(JSON.stringify(rpcPayload) + "\n");
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

    // In-memory store for validator wallets generated during init
    if (!global.validatorWallets) {
        global.validatorWallets = [];
    }

    // POST /api/arena/init
    if (req.url === "/api/arena/init" && req.method === "POST") {
        try {
            const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
            const arenaAddr = config.networks.localhost.arenaProcessorAddress;
            const pkiAddr = config.networks.localhost.consensusPkiAddress;
            
            if (!arenaAddr || !pkiAddr) {
                res.writeHead(400, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: "ArenaProcessor or ConsensusPKI not deployed" }));
                return;
            }
            
            const { ethers } = require("ethers");
            const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            const deployer = await provider.getSigner();
            
            // 1. Load 11 validator wallets from stored private keys in config
            global.validatorWallets = [];
            const validatorAddresses = [];
            const pk11Addr = "0xb0279Db6a2F1E01fbC8483FCCef0Be2bC6299cC3";
            const storedKeys = (config.saved_keys && config.saved_keys[pk11Addr]) || (config.default && config.default.keys) || [];
            
            for (let i = 0; i < 11; i++) {
                let wallet;
                if (storedKeys[i]) {
                    wallet = new ethers.Wallet(storedKeys[i], provider);
                } else {
                    wallet = ethers.Wallet.createRandom().connect(provider);
                }
                global.validatorWallets.push(wallet);
                validatorAddresses.push(wallet.address);
            }
            
            // 2. Initialize ConsensusPKI validators
            const pkiContract = new ethers.Contract(pkiAddr, [
                "function initializeValidatorKeys(address[11] keys) external"
            ], deployer);
            
            const txInit = await pkiContract.initializeValidatorKeys(validatorAddresses);
            await txInit.wait();
            
            // 3. Register 5 players (Card 0, 1, 2, 3, 4)
            const arenaContract = new ethers.Contract(arenaAddr, [
                "function registerPlayerYue(uint256 yueCardId) external",
                "function systemEquipQing(uint256 cardId, uint256 pageIdx, uint256 u1, uint256 u2) external",
                "function processBatch(uint256 batchSize) external"
            ], deployer);
            
            // Clear total players in slot 0x200
            await provider.send("anvil_setStorageAt", [arenaAddr, ethers.zeroPadValue(ethers.toBeHex(0x200), 32), ethers.zeroPadValue("0x00", 32)]);
            
            for (let i = 0; i < 5; i++) {
                // Clear owner slot 0x7000 + i to allow clean re-registration in testing
                const ownerSlot = ethers.zeroPadValue(ethers.toBeHex(0x7000 + i), 32);
                await provider.send("anvil_setStorageAt", [arenaAddr, ownerSlot, ethers.zeroPadValue("0x00", 32)]);

                const txReg = await arenaContract.registerPlayerYue(i);
                await txReg.wait();
            }
            
            // 4. Set their 2-bar equipment params using systemEquipQing on-chain method
            const mockBars = [
                { u1: 0, u2: 20 },  // Width: 20
                { u1: 10, u2: 50 }, // Width: 40
                { u1: 5, u2: 80 },  // Width: 75
                { u1: 20, u2: 30 }, // Width: 10
                { u1: 0, u2: 90 }   // Width: 90 (Winner)
            ];
            
            for (let i = 0; i < 5; i++) {
                const txEquip = await arenaContract.systemEquipQing(i, 0x70, mockBars[i].u1, mockBars[i].u2);
                await txEquip.wait();
            }
            
            res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
            res.end(JSON.stringify({
                status: "success",
                validators: validatorAddresses,
                playersRegistered: 5
            }));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
        }
        return;
    }

    // POST /api/arena/set-target
    if (req.url === "/api/arena/set-target" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => body += chunk.toString());
        req.on("end", async () => {
            try {
                const payload = JSON.parse(body);
                const { qingId } = payload;
                if (qingId === undefined) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Missing qingId" }));
                    return;
                }
                
                if (!global.validatorWallets || global.validatorWallets.length < 11) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Validators not initialized. Run POST /api/arena/init first." }));
                    return;
                }
                
                const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
                const arenaAddr = config.networks.localhost.arenaProcessorAddress;
                const pkiAddr = config.networks.localhost.consensusPkiAddress;
                
                const { ethers } = require("ethers");
                const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
                const deployer = await provider.getSigner();
                
                // 1. Create message hash
                const msgHash = ethers.solidityPackedKeccak256(["uint256"], [qingId]);
                
                // 2. Sign message hash directly (raw, without prefix)
                const signatures = [];
                for (let i = 0; i < 11; i++) {
                    const signature = global.validatorWallets[i].signingKey.sign(msgHash);
                    const sig = ethers.Signature.from(signature).serialized;
                    signatures.push(sig);
                }
                
                // 3. Format setTargetQingViaPKI raw calldata
                const selector = "0x5e2cf9f1";
                const pkiAddrPadded = ethers.zeroPadValue(pkiAddr, 32);
                const qingIdPadded = ethers.zeroPadValue(ethers.toBeHex(qingId), 32);
                const msgHashPadded = ethers.zeroPadValue(msgHash, 32);
                
                const sigsEncoded = ethers.AbiCoder.defaultAbiCoder().encode(["bytes[]"], [signatures]);
                const sigsRaw = ethers.getBytes(sigsEncoded).slice(32);
                
                const txCalldata = ethers.concat([
                    selector,
                    pkiAddrPadded,
                    qingIdPadded,
                    msgHashPadded,
                    sigsRaw
                ]);
                
                const tx = await deployer.sendTransaction({
                    to: arenaAddr,
                    data: txCalldata,
                    gasLimit: 30000000
                });
                await tx.wait();
                
                res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
                res.end(JSON.stringify({
                    status: "success",
                    txHash: tx.hash,
                    msgHash: msgHash
                }));
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }

    // POST /api/arena/process-batch
    if (req.url === "/api/arena/process-batch" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => body += chunk.toString());
        req.on("end", async () => {
            try {
                const payload = JSON.parse(body);
                const { batchSize } = payload;
                if (batchSize === undefined) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Missing batchSize" }));
                    return;
                }
                
                const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
                const arenaAddr = config.networks.localhost.arenaProcessorAddress;
                
                const { ethers } = require("ethers");
                const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
                const deployer = await provider.getSigner();
                
                const arenaContract = new ethers.Contract(arenaAddr, [
                    "function processBatch(uint256 batchSize) external returns (uint256, uint256, uint256)"
                ], deployer);
                
                const tx = await arenaContract.processBatch(batchSize);
                await tx.wait();
                
                const [targetQing, cursor, leader, maxWidth, completed] = await Promise.all([
                    provider.getStorage(arenaAddr, "0x100"),
                    provider.getStorage(arenaAddr, "0x101"),
                    provider.getStorage(arenaAddr, "0x102"),
                    provider.getStorage(arenaAddr, "0x103"),
                    provider.getStorage(arenaAddr, "0x104")
                ]);
                
                res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
                res.end(JSON.stringify({
                    status: "success",
                    txHash: tx.hash,
                    cursor: parseInt(cursor, 16) || 0,
                    leader: parseInt(leader, 16) === 0xFFFFFFFF ? null : parseInt(leader, 16),
                    maxWidth: parseInt(maxWidth, 16) || 0,
                    completed: parseInt(completed, 16) || 0
                }));
            } catch (err) {
                res.writeHead(500, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: err.message }));
            }
        });
        return;
    }

    // GET /api/arena/status
    if (req.url === "/api/arena/status" && req.method === "GET") {
        try {
            const config = JSON.parse(fs.readFileSync(CONFIG_PATH, "utf8"));
            const arenaAddr = config.networks.localhost.arenaProcessorAddress;
            const pkiAddr = config.networks.localhost.consensusPkiAddress;
            
            if (!arenaAddr) {
                res.writeHead(400, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: "ArenaProcessor not deployed" }));
                return;
            }
            
            const { ethers } = require("ethers");
            const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
            
            const [targetQing, cursor, leader, maxWidth, completed, totalPlayers] = await Promise.all([
                provider.getStorage(arenaAddr, "0x100"),
                provider.getStorage(arenaAddr, "0x101"),
                provider.getStorage(arenaAddr, "0x102"),
                provider.getStorage(arenaAddr, "0x103"),
                provider.getStorage(arenaAddr, "0x104"),
                provider.getStorage(arenaAddr, "0x200")
            ]);
            
            res.writeHead(200, { "Content-Type": "application/json", "Access-Control-Allow-Origin": "*" });
            res.end(JSON.stringify({
                targetQing: parseInt(targetQing, 16) || 0,
                cursor: parseInt(cursor, 16) || 0,
                leader: parseInt(leader, 16) === 0xFFFFFFFF ? null : parseInt(leader, 16),
                maxWidth: parseInt(maxWidth, 16) || 0,
                completed: parseInt(completed, 16) || 0,
                totalPlayers: parseInt(totalPlayers, 16) || 0,
                arenaAddress: arenaAddr,
                pkiAddress: pkiAddr
            }));
        } catch (err) {
            res.writeHead(500, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: err.message }));
        }
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
    } else if (parsedUrl.pathname === "/config/live_quaternion_data.json") {
        absolutePath = path.join(__dirname, "../config/live_quaternion_data.json");
    } else if (parsedUrl.pathname === "/scripts/deployed_addresses_localhost.json") {
        absolutePath = path.join(__dirname, "../scripts/deployed_addresses_localhost.json");
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

// Setup automatic relationship spidering when any lore file is modified, added, or deleted
let spiderTimeout = null;
const lorePath = path.join(__dirname, "../lore");
if (fs.existsSync(lorePath)) {
    fs.watch(lorePath, (eventType, filename) => {
        if (filename && (filename.endsWith(".md") || filename.endsWith(".lore"))) {
            clearTimeout(spiderTimeout);
            spiderTimeout = setTimeout(() => {
                try {
                    console.log(`[SERVER] Auto-spidering changes in lore/${filename}...`);
                    spider();
                    buildGlossary();
                } catch (err) {
                    console.error("[SERVER] Auto-spidering error:", err.message);
                }
            }, 500);
        }
    });
}

server.listen(PORT, "127.0.0.1", () => {
    console.log(`==================================================`);
    console.log(`DApp Dashboard Server running at http://127.0.0.1:${PORT}`);
    console.log(`==================================================`);
});
