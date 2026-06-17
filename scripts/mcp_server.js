const fs = require("fs");
const path = require("path");
const http = require("http");

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
            if (!params || params.name !== "render_card") {
                sendError(id, -32601, "Method not found: " + (params ? params.name : ""));
                break;
            }
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
