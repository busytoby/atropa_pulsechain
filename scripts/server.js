const http = require("http");
const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");

const PORT = 3000;
const CONFIG_PATH = path.join(__dirname, "../config/user_config.json");
let dilemmaLog = [];

class AlsaSynth {
    constructor() {
        this.sampleRate = 22050;
        this.activeNotes = new Map();
        this.interval = null;
        this.startLoop();
    }

    noteOn(freq) {
        if (!this.activeNotes.has(freq)) {
            this.activeNotes.set(freq, {
                phase: 0,
                amplitude: 0.0,
                targetAmplitude: 0.15
            });
        } else {
            this.activeNotes.get(freq).targetAmplitude = 0.15;
        }
    }

    noteOff(freq) {
        const note = this.activeNotes.get(freq);
        if (note) {
            note.targetAmplitude = 0.0;
        }
    }

    reset() {
        this.activeNotes.clear();
    }

    startLoop() {
        const blockSize = Math.round(this.sampleRate * 0.02);

        this.interval = setInterval(() => {
            for (let i = 0; i < blockSize; i++) {
                for (const [freq, note] of this.activeNotes.entries()) {
                    note.amplitude += (note.targetAmplitude - note.amplitude) * 0.08;
                    note.phase += (2 * Math.PI * freq) / this.sampleRate;
                    if (note.phase > 2 * Math.PI) {
                        note.phase -= 2 * Math.PI;
                    }
                    
                    if (note.targetAmplitude === 0.0 && note.amplitude < 0.001) {
                        this.activeNotes.delete(freq);
                    }
                }
            }
        }, 20);
    }
}

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

console.log(`[SERVER] Spawning native ZMM VM MCP server: ${mcpBinary}`);
const mcpProcess = spawn(mcpBinary, [], {
    cwd: mcpCwd,
    stdio: "inherit",
    env: {
        ...process.env,
        TSFI_DEFAULT_PKI_KEY: defaultPkiKey
    }
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
let currentNoNukesPriceUsd = 1.74;

const decimalsCache = {
    "0x174a0ad99c60c20d9b3d94c3095bc1fb9defd62": 18
};

function pulseRpcCall(to, data) {
    return new Promise((resolve) => {
        const postData = JSON.stringify({
            jsonrpc: "2.0",
            method: "eth_call",
            params: [
                {
                    to: to,
                    data: data
                },
                "latest"
            ],
            id: 1
        });

        const options = {
            hostname: "rpc.pulsechain.com",
            port: 443,
            path: "/",
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": postData.length
            },
            timeout: 5000
        };

        const req = https.request(options, (res) => {
            let body = "";
            res.on("data", chunk => body += chunk);
            res.on("end", () => {
                try {
                    const parsed = JSON.parse(body);
                    if (parsed.error) {
                        resolve(null);
                    } else {
                        resolve(parsed.result);
                    }
                } catch (e) {
                    resolve(null);
                }
            });
        });

        req.on("error", () => resolve(null));
        req.write(postData);
        req.end();
    });
}

async function getTokenDecimals(tokenAddress) {
    const addr = tokenAddress.toLowerCase();
    if (decimalsCache[addr] !== undefined) {
        return decimalsCache[addr];
    }
    let decimals = 18;
    try {
        const result = await pulseRpcCall(tokenAddress, "0x313ce567");
        if (result && result !== "0x") {
            const parsed = parseInt(result, 16);
            if (!isNaN(parsed)) {
                decimals = parsed;
            }
        }
    } catch (e) {
        // Fallback
    }
    decimalsCache[addr] = decimals;
    return decimals;
}

async function queryTokenMarketData(partnerAddr, poolAddresses, nonukesPriceUsd) {
    try {
        let totalReservePartner = 0;
        let totalReserveNoNukes = 0;
        
        const nonukesDecimals = 18;

        // Try to load local scratch reserves file as a fallback
        let reservesFile = {};
        const glob = require("glob");
        const os = require("os");
        const fs = require("fs");
        const path = require("path");
        try {
            const reservesFiles = glob.sync(os.homedir() + "/.gemini/antigravity-cli/brain/*/scratch/nonukes_pulsex_reserves.json");
            const resPath = reservesFiles[0] || "nonukes_pulsex_reserves.json";
            if (fs.existsSync(resPath)) {
                reservesFile = JSON.parse(fs.readFileSync(resPath, "utf8"));
            }
        } catch (err) {
            // Ignore
        }
        
        let partnerDecimals = 18;
        let decimalsFetched = false;
        
        // Try to get decimals from RPC
        try {
            const result = await pulseRpcCall(partnerAddr, "0x313ce567");
            if (result && result !== "0x") {
                const parsed = parseInt(result, 16);
                if (!isNaN(parsed)) {
                    partnerDecimals = parsed;
                    decimalsFetched = true;
                }
            }
        } catch (e) {
            // Ignore
        }
        
        // If RPC failed to fetch decimals, try to derive it from the reserves file
        if (!decimalsFetched) {
            for (const poolAddr of poolAddresses) {
                const poolKey = poolAddr.toLowerCase();
                if (reservesFile[poolKey]) {
                    const resRecord = reservesFile[poolKey];
                    let rawR = 0n;
                    let adjR = 0;
                    
                    const fileToken0 = (resRecord.token0 || "").toLowerCase();
                    const fileToken1 = (resRecord.token1 || "").toLowerCase();
                    if (fileToken0 === partnerAddr.toLowerCase()) {
                        rawR = BigInt(resRecord.raw_reserve0 || "0");
                        adjR = Number(resRecord.reserve0 || 0);
                    } else if (fileToken1 === partnerAddr.toLowerCase()) {
                        rawR = BigInt(resRecord.raw_reserve1 || "0");
                        adjR = Number(resRecord.reserve1 || 0);
                    }
                    
                    if (rawR > 0n && adjR > 0) {
                        const ratio = Number(rawR) / adjR;
                        const derivedDec = Math.round(Math.log10(ratio));
                        if (derivedDec >= 0 && derivedDec <= 36) {
                            partnerDecimals = derivedDec;
                            decimalsFetched = true;
                            break;
                        }
                    }
                }
            }
        }
        
        for (const poolAddr of poolAddresses) {
            const poolKey = poolAddr.toLowerCase();
            let r0 = 0n;
            let r1 = 0n;
            let fetched = false;

            const reservesResult = await pulseRpcCall(poolAddr, "0x0902f1ac");
            if (reservesResult && reservesResult !== "0x") {
                const clean = reservesResult.startsWith("0x") ? reservesResult.slice(2) : reservesResult;
                if (clean.length >= 128) {
                    const r0Hex = clean.slice(0, 64);
                    const r1Hex = clean.slice(64, 128);
                    r0 = BigInt("0x" + r0Hex);
                    r1 = BigInt("0x" + r1Hex);
                    fetched = true;
                }
            }

            let reservePartner = 0;
            let reserveNoNukes = 0;

            if (fetched) {
                const nonukesAddr = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62";
                const isPartner0 = partnerAddr.toLowerCase() < nonukesAddr.toLowerCase();
                
                const rawReservePartner = isPartner0 ? r0 : r1;
                const rawReserveNoNukes = isPartner0 ? r1 : r0;
                
                reservePartner = Number(rawReservePartner) / Math.pow(10, partnerDecimals);
                reserveNoNukes = Number(rawReserveNoNukes) / Math.pow(10, nonukesDecimals);
            } else if (reservesFile[poolKey]) {
                const resRecord = reservesFile[poolKey];
                const fileR0 = Number(resRecord.reserve0 || 0);
                const fileR1 = Number(resRecord.reserve1 || 0);
                
                let fileToken0 = (resRecord.token0 || "").toLowerCase();
                let fileToken1 = (resRecord.token1 || "").toLowerCase();
                if (!fileToken0 || !fileToken1) {
                    if (partnerAddr.toLowerCase() < "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62") {
                        fileToken0 = partnerAddr.toLowerCase();
                        fileToken1 = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62";
                    } else {
                        fileToken0 = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62";
                        fileToken1 = partnerAddr.toLowerCase();
                    }
                }
                
                const nonukesAddr = "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62";
                if (fileToken0 === nonukesAddr) {
                    reserveNoNukes = fileR0;
                    reservePartner = fileR1 * Math.pow(10, 18 - partnerDecimals);
                } else {
                    reserveNoNukes = fileR1;
                    reservePartner = fileR0 * Math.pow(10, 18 - partnerDecimals);
                }
            }
            
            totalReservePartner += reservePartner;
            totalReserveNoNukes += reserveNoNukes;
        }
        
        let priceUsd = "0";
        if (totalReservePartner > 0) {
            const priceInNoNukes = totalReserveNoNukes / totalReservePartner;
            priceUsd = (priceInNoNukes * nonukesPriceUsd).toFixed(6);
        }
        
        const liquidityUsd = Math.round(totalReserveNoNukes * nonukesPriceUsd * 2);
        
        return {
            priceUsd,
            liquidityUsd
        };
    } catch (e) {
        console.error(`[MARKET CACHE] Error querying live data for ${partnerAddr}:`, e.message);
        return { priceUsd: "0", liquidityUsd: 0 };
    }
}

function fetchMarketData(addresses) {
    if (marketFetchInProgress || !addresses || addresses.length === 0) return;
    marketFetchInProgress = true;
    console.log(`[MARKET CACHE] Starting update using PulseX V1 & V2 on-chain reserves for ${addresses.length} tokens...`);

    const nonukesPriceUsd = 1.74;
    console.log(`[MARKET CACHE] Current NoNukes USD price: ${nonukesPriceUsd}`);
    currentNoNukesPriceUsd = nonukesPriceUsd;

    // Immediately execute reserves updates using the known price
    (async () => {
        try {

            let poolsMap = {};
            const poolsPath = path.join(__dirname, "../nonukes_pools.json");
            if (fs.existsSync(poolsPath)) {
                try {
                    poolsMap = JSON.parse(fs.readFileSync(poolsPath, "utf8"));
                } catch (e) {
                    console.error("[MARKET CACHE] Failed to read nonukes_pools.json:", e.message);
                }
            }

            const partnerToPools = {};
            for (const [poolAddr, poolInfo] of Object.entries(poolsMap)) {
                if (poolInfo.other_addr) {
                    const partnerAddr = poolInfo.other_addr.toLowerCase();
                    if (!partnerToPools[partnerAddr]) {
                        partnerToPools[partnerAddr] = [];
                    }
                    partnerToPools[partnerAddr].push(poolAddr.toLowerCase());
                }
            }

            const marketData = {};
            const batchSize = 10;
            
            try {
                for (let i = 0; i < addresses.length; i += batchSize) {
                    const batch = addresses.slice(i, i + batchSize);
                    await Promise.all(batch.map(async (addr) => {
                        const addrKey = addr.toLowerCase();
                        const poolsList = partnerToPools[addrKey];
                        
                        if (poolsList && poolsList.length > 0) {
                            const metrics = await queryTokenMarketData(addr, poolsList, nonukesPriceUsd);
                            marketData[addrKey] = {
                                priceUsd: metrics.priceUsd,
                                priceChange24h: 0,
                                liquidityUsd: metrics.liquidityUsd,
                                dexId: "pulsex",
                                quoteSymbol: "NONUKES"
                            };
                        } else {
                            marketData[addrKey] = {
                                priceUsd: "0",
                                priceChange24h: 0,
                                liquidityUsd: 0,
                                dexId: "unknown",
                                quoteSymbol: "NONUKES"
                            };
                        }
                    }));
                }

                fs.mkdirSync(path.dirname(MARKET_CACHE_PATH), { recursive: true });
                fs.writeFileSync(MARKET_CACHE_PATH, JSON.stringify({
                    timestamp: Date.now(),
                    data: marketData
                }, null, 2), "utf8");
                console.log(`[MARKET CACHE] Calculated and saved ${Object.keys(marketData).length} token price records directly from PulseX.`);
            } catch (err) {
                console.error("[MARKET CACHE] Failed to fetch or write live cache data:", err);
            }
            marketFetchInProgress = false;
        } catch (outerErr) {
            console.error("[MARKET CACHE] Unexpected error during market data fetch:", outerErr);
            marketFetchInProgress = false;
        }
    })();
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

let loreAnalysisCache = {};

function performLoreAnalysis() {
    const loreDir = path.join(__dirname, "../lore");
    if (!fs.existsSync(loreDir)) return;
    const files = fs.readdirSync(loreDir).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
    
    // Normalize defined concepts to a map and set
    const conceptMap = {};
    files.forEach(f => {
        const base = path.basename(f, path.extname(f)).toLowerCase().replace(/_/g, ' ');
        conceptMap[base] = f;
    });
    const definedConcepts = new Set(Object.keys(conceptMap));
    
    const rawData = {};
    
    // PHASE 1 & PHASE 2: Structural and Lexical analysis pass
    files.forEach(file => {
        const fullPath = path.join(loreDir, file);
        try {
            const text = fs.readFileSync(fullPath, "utf8");
            
            const annotationsRegex = /<!--\s*AUNCIENT_ANNOTATIONS_START\s*([\s\S]*?)\s*AUNCIENT_ANNOTATIONS_END\s*-->/;
            const cleanText = text.replace(annotationsRegex, "");
            
            // Phase 1: Structural analysis
            const words = cleanText.split(/\s+/).filter(Boolean);
            const wordCount = words.length;
            
            const sentences = cleanText.split(/[.!?]+/).filter(s => s.trim().length > 0);
            const sentenceCount = sentences.length;
            
            let questionCount = 0;
            const match = text.match(annotationsRegex);
            if (match) {
                try {
                    const annotations = JSON.parse(match[1].trim());
                    annotations.forEach(ann => {
                        if (ann.comment && ann.comment.includes('?')) {
                            questionCount++;
                        }
                    });
                } catch (e) {}
            }
            
            const contentQuestions = (cleanText.match(/\?/g) || []).length;
            const codeBlockCount = (cleanText.match(/```/g) || []).length / 2;
            const headerCount = (cleanText.match(/^#{1,6}\s+/gm) || []).length;
            
            // Phase 2: Lexical Analysis & Jargon Extraction
            const jargonMatches = cleanText.match(/\b[A-Z][a-zA-Z0-9_]{2,}\b/g) || [];
            const uniqueJargon = [...new Set(jargonMatches)];
            const undefinedJargon = [];
            
            uniqueJargon.forEach(term => {
                const normalized = term.toLowerCase();
                if (!definedConcepts.has(normalized) && 
                    !['the', 'and', 'for', 'this', 'that', 'with', 'evm', 'pki', 'gdk', 'gtk', 'xml', 'api', 'rpc', 'sdk', 'url', 'uri', 'web', 'dom', 'cdp', 'cli', 'pid', 'sigint', 'sigterm', 'json', 'html', 'css', 'git', 'vcs', 'mjpeg', 'vulkan', 'wayland'].includes(normalized)) {
                    undefinedJargon.push(term);
                }
            });
            
            const todoMatches = cleanText.match(/\b(TODO|FIXME|TBD|PLACEHOLDER|UNRESOLVED|INVESTIGATE|DRAFT)\b/gi) || [];
            const todoCount = todoMatches.length;

            rawData[file] = {
                file,
                text: cleanText.toLowerCase(),
                wordCount,
                sentenceCount,
                questionCount: questionCount + contentQuestions,
                codeBlockCount,
                headerCount,
                undefinedJargon,
                todoCount,
                referencedConcepts: []
            };
        } catch (e) {}
    });

    // PHASE 3: Relationship Mapping and Concept Reference Graph
    Object.keys(rawData).forEach(file => {
        const data = rawData[file];
        definedConcepts.forEach(concept => {
            const conceptFile = conceptMap[concept];
            if (conceptFile === file) return;
            
            const regex = new RegExp(`\\b${concept}\\b`, 'i');
            if (regex.test(data.text)) {
                data.referencedConcepts.push(conceptFile);
            }
        });
    });

    // PHASE 4: Score Aggregation and Weight Synthesis
    const baseAnalysis = {};
    Object.keys(rawData).forEach(file => {
        const data = rawData[file];
        let score = 0;
        const reasons = [];

        if (data.wordCount < 150) {
            score += 100;
            reasons.push(`Sparse content (${data.wordCount} words)`);
        } else if (data.wordCount < 400) {
            score += 30;
            reasons.push(`Short document (${data.wordCount} words)`);
        }
        
        if (data.headerCount === 0) {
            score += 25;
            reasons.push(`Lacks proper Markdown headers`);
        }

        if (data.questionCount > 0) {
            score += data.questionCount * 40;
            reasons.push(`Contains ${data.questionCount} unresolved question(s)`);
        }

        if (data.undefinedJargon.length > 0) {
            const added = Math.min(5, data.undefinedJargon.length) * 15;
            score += added;
            reasons.push(`Undefined terms: ${data.undefinedJargon.slice(0, 3).join(', ')}`);
        }

        if (data.referencedConcepts.length === 0) {
            score += 40;
            reasons.push(`Isolated node (references no other Auncient concepts)`);
        } else {
            const linkDensity = data.referencedConcepts.length / (data.wordCount / 100);
            if (linkDensity < 0.5) {
                score += 20;
                reasons.push(`Low linkage density to other concepts`);
            }
        }

        if (data.todoCount > 0) {
            score += Math.min(4, data.todoCount) * 25;
            reasons.push(`Contains ${data.todoCount} draft/TODO marker(s)`);
        }

        baseAnalysis[file] = {
            score,
            reasons,
            wordCount: data.wordCount,
            undefinedJargon: data.undefinedJargon,
            referencedConcepts: data.referencedConcepts
        };
    });

    // Pass 2: Propagate weights through concept references to penalize dependency gaps
    const finalAnalysis = {};
    Object.keys(baseAnalysis).forEach(file => {
        const item = baseAnalysis[file];
        let score = item.score;
        const reasons = [...item.reasons];
        
        const weakReferences = [];
        item.referencedConcepts.forEach(refFile => {
            const refItem = baseAnalysis[refFile];
            if (refItem && refItem.score >= 100) {
                weakReferences.push(refFile);
            }
        });

        if (weakReferences.length > 0) {
            const added = Math.min(4, weakReferences.length) * 15;
            score += added;
            const names = weakReferences.slice(0, 2).map(f => path.basename(f, '.md'));
            reasons.push(`Depends on poorly understood concepts: ${names.join(', ')}`);
        }

        finalAnalysis[file] = {
            score,
            reasons,
            wordCount: item.wordCount,
            undefinedJargon: item.undefinedJargon,
            referencedConcepts: item.referencedConcepts
        };
    });
    
    loreAnalysisCache = finalAnalysis;
}

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
        res.end(JSON.stringify({ priceUsd: currentNoNukesPriceUsd }));
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

    // API endpoint to list lore documentation files
    if (req.url === "/api/lore/list") {
        const loreDir = path.join(__dirname, "../lore");
        try {
            if (fs.existsSync(loreDir)) {
                // Execute git status to check for untracked or modified files under lore/
                let gitStatusMap = {};
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

                const files = fs.readdirSync(loreDir);
                const results = files
                    .filter(file => file.endsWith(".md") || file.endsWith(".lore"))
                    .map(file => {
                        const fullPath = path.join(loreDir, file);
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
                            gitStatus: gitStatusMap[file] || null,
                            reviewed: isReviewed,
                            louScore: (loreAnalysisCache[file] ? loreAnalysisCache[file].score : 0),
                            louReasons: (loreAnalysisCache[file] ? loreAnalysisCache[file].reasons : [])
                        };
                    });
                res.writeHead(200, {
                    "Content-Type": "application/json",
                    "Access-Control-Allow-Origin": "*"
                });
                res.end(JSON.stringify(results));
            } else {
                res.writeHead(404, { "Content-Type": "application/json" });
                res.end(JSON.stringify({ error: "Lore directory not found" }));
            }
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
        if (!fileName) {
            res.writeHead(400, { "Content-Type": "application/json" });
            res.end(JSON.stringify({ error: "File parameter missing" }));
            return;
        }
        const cleanName = path.basename(fileName);
        const filePath = path.join(__dirname, "../lore", cleanName);
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

function processAnnotationReplies(content, file) {
    const regex = /<!--\s*AUNCIENT_ANNOTATIONS_START\s*([\s\S]*?)\s*AUNCIENT_ANNOTATIONS_END\s*-->/;
    const match = content.match(regex);
    if (!match) return content;
    
    try {
        const annotations = JSON.parse(match[1].trim());
        console.log(`[ANN DEB] file: ${file}, annotations length: ${annotations.length}`);
        let updated = false;
        
        const loreDir = path.join(__dirname, "../lore");
        const files = fs.readdirSync(loreDir).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
        const concepts = files.map(f => ({
            file: f,
            name: path.basename(f, path.extname(f)).toLowerCase().replace(/_/g, ' ')
        }));

        annotations.forEach((ann, idx) => {
            const isQuestion = ann.comment && ann.comment.includes('?');
            const hasNoReplies = !ann.replies || ann.replies.length === 0;
            console.log(`[ANN DEB][${idx}] author: ${ann.author}, comment: "${ann.comment}", isQuestion: ${isQuestion}, hasNoReplies: ${hasNoReplies}`);
            
            if (ann.author === 'User' && isQuestion && hasNoReplies) {
                if (!ann.replies) ann.replies = [];
                
                let replyText = "";
                const textToSearch = (ann.comment + " " + (ann.range ? content.substring(ann.range[0], ann.range[1]) : "")).toLowerCase();
                const cleanSearch = textToSearch.replace(/[\s_-]/g, '');
                console.log(`[ANN DEB][${idx}] cleanSearch: "${cleanSearch}"`);
                
                const matchingConcept = concepts.find(c => {
                    const cleanConcept = c.name
                        .replace(/\b(lore|spec|analysis|integration|roadmap|proposal|deep\s+dive|guide|blueprint|report)\b/g, '')
                        .trim()
                        .replace(/[\s_-]/g, '');
                    if (!cleanConcept) return false;
                    return cleanSearch.includes(cleanConcept);
                });
                console.log(`[ANN DEB][${idx}] matchingConcept:`, matchingConcept ? matchingConcept.file : "none");

                if (matchingConcept) {
                    try {
                        const targetPath = path.join(loreDir, matchingConcept.file);
                        const targetText = fs.readFileSync(targetPath, "utf8");
                        const cleanTarget = targetText.replace(/<!--[\s\S]*?-->/g, "").trim();
                        const paragraph = cleanTarget.split("\n\n")[0] || cleanTarget;
                        const sentences = paragraph.split(/[.!?]+/).slice(0, 2).join(". ");
                        replyText = `Based on Auncient lore in ${matchingConcept.file}: "${sentences.trim()}."`;
                    } catch (e) {
                        replyText = `Found reference to ${matchingConcept.file}, but could not load definition.`;
                    }
                } else {
                    replyText = `I detected a question about this reference, but I do not have a defined Auncient concept for it. This remains a knowledge gap.`;
                }
                
                ann.replies.push({
                    author: "AI",
                    comment: replyText,
                    timestamp: Date.now()
                });
                updated = true;
            }
        });
        
        if (updated) {
            const newBlock = `<!-- AUNCIENT_ANNOTATIONS_START\n${JSON.stringify(annotations, null, 2)}\nAUNCIENT_ANNOTATIONS_END -->`;
            return content.replace(regex, newBlock);
        }
    } catch (err) {
        console.error("[SERVER] Failed to process annotation replies:", err);
    }
    return content;
}

    // API endpoint to write/save a lore file content
    if (req.url === "/api/lore/save" && req.method === "POST") {
        let body = "";
        req.on("data", chunk => {
            body += chunk.toString();
        });
        req.on("end", () => {
            try {
                const payload = JSON.parse(body);
                const { file, content } = payload;
                if (!file || content === undefined) {
                    res.writeHead(400, { "Content-Type": "application/json" });
                    res.end(JSON.stringify({ error: "Invalid payload parameters" }));
                    return;
                }
                const cleanName = path.basename(file);
                const filePath = path.join(__dirname, "../lore", cleanName);
                
                // Process automated replies to annotations
                const processedContent = processAnnotationReplies(content, cleanName);
                
                fs.writeFileSync(filePath, processedContent, "utf8");
                try {
                    performLoreAnalysis();
                } catch (e) {}

                // Auto-commit saved lore file safely without staging any binaries
                try {
                    const { execSync } = require("child_process");
                    execSync(`git add lore/${cleanName}`, { cwd: path.join(__dirname, "..") });
                    execSync(`git commit -m "Auto-commit lore: ${cleanName}"`, { cwd: path.join(__dirname, "..") });
                    console.log(`[SERVER] Auto-committed lore/${cleanName}`);
                } catch (gitErr) {
                    console.error("[SERVER] Failed to auto-commit lore file:", gitErr.message);
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
            const files = [
                path.join(__dirname, "../found_addresses.txt"),
                "/home/mariarahel/repkeys.txt",
                "/home/mariarahel/repkeys2.txt"
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
                    let rpcPayload;
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
                    socket.write(JSON.stringify(rpcPayload));
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
