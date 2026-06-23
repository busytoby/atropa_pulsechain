const fs = require("fs");
const path = require("path");
const http = require("http");
const net = require("net");

// Simple JSON-RPC 2.0 stdio-based MCP Server
const serverInfo = {
    name: "tsfi-card-renderer",
    version: "1.0.0"
};

const tools = [
    {
        name: "render_card",
        description: "Synthesize and render a 1980s retro-futuristic cel-shaded sci-fi trading card game art based on a description prompt.",
        inputSchema: {
            type: "object",
            properties: {
                prompt: {
                    type: "string",
                    description: "Description of the card artwork to generate (e.g. 'Gold shiny coin, neon glowing cyber aesthetic, vector outline')"
                },
                steps: {
                    type: "number",
                    description: "Number of denoising steps (default: 20 for masterpiece quality, 4 for draft)"
                },
                cfg: {
                    type: "number",
                    description: "Classifier Free Guidance scale (default: 7.5)"
                },
                address: {
                    type: "string",
                    description: "Optional smart contract address to load DNA color specs and on-chain metrics"
                }
            },
            required: ["prompt"]
        }
    },
    {
        name: "get_chats",
        description: "Retrieve all monitored Void and QING chat history from the PulseChain event stream.",
        inputSchema: {
            type: "object",
            properties: {
                limit: {
                    type: "number",
                    description: "Limit the number of recent chat messages returned (default: all)"
                }
            }
        }
    },
    {
        name: "get_qings",
        description: "Retrieve all registered QING contracts on PulseChain with their symbols and contract addresses.",
        inputSchema: {
            type: "object",
            properties: {
                symbol: {
                    type: "string",
                    description: "Optional symbol filter to find a specific QING"
                }
            }
        }
    },
    {
        name: "get_prices",
        description: "Retrieve stable and regular price data for PulseChain tokens from the local price cache.",
        inputSchema: {
            type: "object",
            properties: {
                symbol: {
                    type: "string",
                    description: "Optional symbol filter to find a specific token's price (e.g. 'WPLS')"
                }
            }
        }
    },
    {
        name: "query_lore",
        description: "Query the relational lore database. Search for documents matching a specific tag, find tags linked to a specific document, or query on-chain Yul QING relationships.",
        inputSchema: {
            type: "object",
            properties: {
                tag: {
                    type: "string",
                    description: "Optional: Find all documents associated with this tag."
                },
                document: {
                    type: "string",
                    description: "Optional: Find all tags associated with this document."
                },
                qingAddress: {
                    type: "string",
                    description: "Optional: Query live on-chain tags mapped to this contract address from the Yul Arena CPU."
                }
            }
        }
    },
    {
        name: "log_youtube_telemetry",
        description: "Log played YouTube video telemetry (video ID and title) to the persistent record.",
        inputSchema: {
            type: "object",
            properties: {
                videoId: {
                    type: "string",
                    description: "The YouTube video ID (the v= parameter value)"
                },
                title: {
                    type: "string",
                    description: "The title of the YouTube video"
                }
            },
            required: ["videoId", "title"]
        }
    },
    {
        name: "get_youtube_telemetry",
        description: "Retrieve the list of logged YouTube video playback records.",
        inputSchema: {
            type: "object",
            properties: {}
        }
    },
    {
        name: "search_lore_context",
        description: "Search for a specific word or multi-word term across all lore documents and retrieve matching line context/snippets.",
        inputSchema: {
            type: "object",
            properties: {
                term: {
                    type: "string",
                    description: "The term or sequence to search for (e.g. 'zmm vm')"
                }
            },
            required: ["term"]
        }
    }
];

function log(msg) {
    fs.writeSync(2, `[MCP Server] ${msg}\n`);
}

// Process stdin messages line-by-line
let buffer = "";
process.stdin.on("data", chunk => {
    buffer += chunk.toString();
    const lines = buffer.split("\n");
    buffer = lines.pop(); // keep incomplete line
    
    for (const line of lines) {
        if (!line.trim()) continue;
        try {
            const request = JSON.parse(line);
            handleRequest(request);
        } catch (err) {
            sendError(null, -32700, "Parse error: " + err.message);
        }
    }
});

function sendResponse(id, result) {
    process.stdout.write(JSON.stringify({
        jsonrpc: "2.0",
        id,
        result
    }) + "\n");
}

function sendError(id, code, message) {
    process.stdout.write(JSON.stringify({
        jsonrpc: "2.0",
        id,
        error: { code, message }
    }) + "\n");
}

async function handleRequest(req) {
    const { method, params, id } = req;
    log(`Received method: ${method}`);
    
    switch (method) {
        case "initialize":
            sendResponse(id, {
                protocolVersion: "2024-11-05",
                capabilities: {
                    tools: {}
                },
                serverInfo
            });
            break;
            
        case "initialized":
            // Notification: no response required
            break;
            
        case "tools/list":
            sendResponse(id, { tools });
            break;
            
        case "tools/call":
            if (!params) {
                sendError(id, -32602, "Invalid params");
                break;
            }
            if (params.name === "render_card") {
                try {
                    const args = params.arguments || {};
                    const result = await triggerRender(args);
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: `Card generated successfully!\nArtifact Path: ${result.url}\n\n=== Render Logs ===\n${result.logs}`
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to render card: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "get_chats") {
                try {
                    const args = params.arguments || {};
                    const limit = args.limit;
                    const chats = await fetchChatsFromZmm();
                    let formatted = chats.map(chat => {
                        const time = new Date(chat.timestamp * 1000).toISOString();
                        const type = chat.event === "M:VOID_CHAT" ? "VOID" : "QING";
                        return `[${time}] [${type}] ${chat.source}: ${chat.details}`;
                    });
                    if (limit) {
                        formatted = formatted.slice(-limit);
                    }
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: formatted.length > 0 ? formatted.join("\n") : "No chat history available."
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to retrieve chats: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "get_qings") {
                try {
                    const args = params.arguments || {};
                    const symbolFilter = args.symbol ? args.symbol.toLowerCase() : null;
                    const statusPath = path.join(__dirname, "../config/nonukes_qings_status.json");
                    if (!fs.existsSync(statusPath)) {
                        throw new Error("QING status configuration file not found");
                    }
                    const qingsData = JSON.parse(fs.readFileSync(statusPath, "utf8"));
                    const qings = qingsData.filter(item => item.exists && item.qing);
                    
                    let filtered = qings;
                    if (symbolFilter) {
                        filtered = qings.filter(item => item.symbol.toLowerCase().includes(symbolFilter));
                    }
                    
                    const resultText = filtered.map(item => `${item.symbol}: ${item.qing}`).join("\n");
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: resultText || "No matching QING contracts found."
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to retrieve QINGs: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "get_prices") {
                try {
                    const args = params.arguments || {};
                    const symbolFilter = args.symbol ? args.symbol.toUpperCase() : null;
                    const cachePath = path.join(__dirname, "../price_cache.json");
                    const prices = [];
                    
                    if (fs.existsSync(cachePath)) {
                        const cacheData = JSON.parse(fs.readFileSync(cachePath, "utf8"));
                        for (const [addr, info] of Object.entries(cacheData)) {
                            if (info && typeof info === "object" && info.symbol) {
                                if (!symbolFilter || info.symbol.toUpperCase().includes(symbolFilter)) {
                                    prices.push({
                                        address: addr,
                                        symbol: info.symbol,
                                        name: info.name || "Unknown",
                                        priceUsd: info.price,
                                        source: "oracle_cache"
                                    });
                                }
                            }
                        }
                    }
                    
                    const marketPath = path.join(__dirname, "../tmp/market_cache.json");
                    if (fs.existsSync(marketPath)) {
                        const marketData = JSON.parse(fs.readFileSync(marketPath, "utf8"));
                        if (marketData && marketData.data) {
                            for (const [addr, info] of Object.entries(marketData.data)) {
                                let symbol = "UNKNOWN";
                                let name = "Unknown";
                                const cardPath = path.join(__dirname, `../solidity/dysnomia/domain/data/${addr}.json`);
                                if (fs.existsSync(cardPath)) {
                                    try {
                                        const card = JSON.parse(fs.readFileSync(cardPath, "utf8"));
                                        symbol = card.symbol || symbol;
                                        name = card.name || name;
                                    } catch (e) {}
                                }
                                if (!symbolFilter || symbol.toUpperCase().includes(symbolFilter)) {
                                    prices.push({
                                        address: addr,
                                        symbol: symbol,
                                        name: name,
                                        priceUsd: parseFloat(info.priceUsd || "0"),
                                        liquidityUsd: info.liquidityUsd || 0,
                                        source: "market_reserves"
                                    });
                                }
                            }
                        }
                    }
                    
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: JSON.stringify(prices, null, 2)
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to retrieve prices: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "query_lore") {
                try {
                    const args = params.arguments || {};
                    const tagQuery = args.tag ? args.tag.toLowerCase() : null;
                    const docQuery = args.document ? args.document.toLowerCase() : null;
                    const qingAddress = args.qingAddress ? args.qingAddress.toLowerCase() : null;
                    const dbPath = path.join(__dirname, "../config/lore_tags_database.json");
                    if (!fs.existsSync(dbPath)) {
                        throw new Error("Lore tag database not found. Please run scripts/spider_lore_tags.js first.");
                    }
                    const db = JSON.parse(fs.readFileSync(dbPath, "utf8"));
                    let result = null;

                    if (qingAddress) {
                        const { ethers } = require("ethers");
                        const configPath = path.join(__dirname, "../config/user_config.json");
                        if (!fs.existsSync(configPath)) {
                            throw new Error("User configuration file not found");
                        }
                        const configData = JSON.parse(fs.readFileSync(configPath, "utf8"));
                        const arenaAddr = configData.networks.localhost.arenaProcessorAddress;
                        if (!arenaAddr) {
                            throw new Error("ArenaProcessor address not found in config");
                        }

                        const provider = new ethers.JsonRpcProvider("http://127.0.0.1:8545");
                        const arenaContract = new ethers.Contract(arenaAddr, [
                            "function tags(address qing) external view returns (address[])"
                        ], provider);

                        const taggedAddresses = await arenaContract.tags(qingAddress);
                        
                        // Map each address back to documents using the relational database
                        const mappedDocs = [];
                        if (taggedAddresses) {
                            for (const addr of taggedAddresses) {
                                const cleanAddr = addr.toLowerCase();
                                const matchedQing = Object.keys(db.qingToDocuments).find(k => k.toLowerCase() === cleanAddr);
                                const docs = matchedQing ? db.qingToDocuments[matchedQing] : [];
                                mappedDocs.push({ address: addr, documents: docs });
                            }
                        }
                        result = {
                            qing: qingAddress,
                            onChainTags: taggedAddresses,
                            mappedDocuments: mappedDocs
                        };
                    } else if (tagQuery) {
                        result = db.tagToDocuments[tagQuery] || [];
                    } else if (docQuery) {
                        const matchedKey = Object.keys(db.documentToTags).find(k => k.toLowerCase() === docQuery || k.toLowerCase().includes(docQuery));
                        result = matchedKey ? { document: matchedKey, tags: db.documentToTags[matchedKey] } : null;
                    } else {
                        result = {
                            meta: db.meta,
                            availableTagsSample: Object.keys(db.tagToDocuments).slice(0, 50)
                        };
                    }

                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: JSON.stringify(result, null, 2)
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to query lore database: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "log_youtube_telemetry") {
                try {
                    const args = params.arguments || {};
                    const { videoId, title } = args;
                    if (!videoId || !title) {
                        throw new Error("Missing videoId or title parameter");
                    }
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
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: `Successfully logged YouTube video: "${title}" (v=${videoId})`
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to log YouTube video: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "get_youtube_telemetry") {
                try {
                    const recordPath = path.join(__dirname, "../tmp/youtube_telemetry.json");
                    let records = [];
                    if (fs.existsSync(recordPath)) {
                        records = JSON.parse(fs.readFileSync(recordPath, "utf8"));
                    }
                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: JSON.stringify(records, null, 2)
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to retrieve YouTube telemetry: ${err.message}`
                            }
                        ]
                    });
                }
            } else if (params.name === "search_lore_context") {
                try {
                    const args = params.arguments || {};
                    const term = args.term;
                    if (!term) {
                        throw new Error("Missing search term");
                    }
                    const loreDir = path.join(__dirname, "../lore");
                    if (!fs.existsSync(loreDir)) {
                        throw new Error("Lore directory not found");
                    }
                    const files = fs.readdirSync(loreDir).filter(f => f.endsWith(".md") || f.endsWith(".lore"));
                    const matches = [];
                    const searchRegex = new RegExp(term.replace(/[-\/\\^$*+?.()|[\]{}]/g, '\\$&'), "i");

                    for (const filename of files) {
                        const filePath = path.join(loreDir, filename);
                        const lines = fs.readFileSync(filePath, "utf8").split(/\r?\n/);
                        for (let i = 0; i < lines.length; i++) {
                            if (searchRegex.test(lines[i])) {
                                const start = Math.max(0, i - 1);
                                const end = Math.min(lines.length - 1, i + 1);
                                const context = lines.slice(start, end + 1).map((l, idx) => `  L${start + idx + 1}: ${l}`).join("\n");
                                matches.push(`[${filename}]\n${context}`);
                            }
                        }
                    }

                    sendResponse(id, {
                        content: [
                            {
                                type: "text",
                                text: matches.length > 0 ? matches.join("\n\n") : `No matches found for term "${term}".`
                            }
                        ]
                    });
                } catch (err) {
                    sendResponse(id, {
                        isError: true,
                        content: [
                            {
                                type: "text",
                                text: `Failed to search lore context: ${err.message}`
                            }
                        ]
                    });
                }
            } else {
                sendError(id, -32601, "Method not found: " + params.name);
            }
            break;
            
        default:
            sendError(id, -32601, `Method not found: ${method}`);
    }
}

function triggerRender(args) {
    return new Promise((resolve, reject) => {
        const payload = JSON.stringify({
            frame: 777,
            g_x: 0.0,
            g_y: 0.0,
            stretch: 1.0,
            pulse: 0.0,
            fire: 0.0,
            light: 1.0,
            steps: args.steps || 20,
            cfg: args.cfg || 7.5,
            promptOverride: args.prompt,
            address: args.address || null
        });

        const req = http.request({
            hostname: "127.0.0.1",
            port: 3000,
            path: "/api/render-dna",
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": Buffer.byteLength(payload)
            }
        }, (res) => {
            let sseData = "";
            res.on("data", (chunk) => {
                sseData += chunk.toString();
            });

            res.on("end", () => {
                // Parse the SSE response to find the success object
                const lines = sseData.split("\n");
                let successResult = null;
                let errorResult = null;

                for (const line of lines) {
                    if (line.startsWith("data: ")) {
                        try {
                            const parsed = JSON.parse(line.slice(6));
                            if (parsed.type === "success") {
                                successResult = parsed;
                            } else if (parsed.type === "error") {
                                errorResult = parsed;
                            }
                        } catch (e) {
                            // ignore parsing issues on SSE metadata
                        }
                    }
                }

                if (successResult) {
                    resolve(successResult);
                } else if (errorResult) {
                    reject(new Error(errorResult.content));
                } else {
                    reject(new Error("Render stream completed without success event."));
                }
            });
        });

        req.on("error", (e) => {
            reject(new Error("Connection to DApp backend failed: " + e.message));
        });

        req.write(payload);
        req.end();
    });
}

function fetchChatsFromZmm() {
    return new Promise((resolve, reject) => {
        const client = new net.Socket();
        client.setTimeout(3000);
        let data = "";
        
        client.connect(10042, "127.0.0.1", () => {
            client.write(JSON.stringify({
                jsonrpc: "2.0",
                method: "wave512.dilemma_log",
                params: {},
                id: 1
            }));
        });
        
        client.on("data", chunk => {
            data += chunk.toString();
        });
        
        client.on("end", () => {
            try {
                const response = JSON.parse(data);
                if (response.error) {
                    reject(new Error(response.error.message));
                } else {
                    const logs = response.result || [];
                    const chatLogs = logs.filter(log => log.event === "M:VOID_CHAT" || log.event === "M:QING_CHAT");
                    resolve(chatLogs);
                }
            } catch (err) {
                reject(new Error("Failed to parse ZMM response: " + err.message));
            }
        });
        
        client.on("timeout", () => {
            client.destroy();
            reject(new Error("ZMM log query timed out"));
        });
        
        client.on("error", err => {
            reject(new Error("ZMM connection error: " + err.message));
        });
    });
}
