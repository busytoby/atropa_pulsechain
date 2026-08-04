const fs = require("fs");
const path = require("path");
const http = require("http");
const { execSync } = require("child_process");
const WebSocket = require("ws");

const TARGET_SCRIPT = "tsfi_live_streamer.js";

function getStreamerPid() {
    try {
        const out = execSync(`pgrep -f ${TARGET_SCRIPT}`).toString().trim();
        const pids = out.split("\n").map(Number).filter(Boolean);
        return pids[0] || null;
    } catch (e) {
        return null;
    }
}

function enableInspector(pid) {
    console.log(`[PATCHER] Activating V8 Inspector on process PID ${pid}...`);
    process.kill(pid, "SIGUSR1");
    // Wait briefly for port to bind
    return new Promise(resolve => setTimeout(resolve, 800));
}

function fetchDebuggerUrl() {
    return new Promise((resolve, reject) => {
        http.get("http://127.0.0.1:9229/json", (res) => {
            let data = "";
            res.on("data", chunk => data += chunk);
            res.on("end", () => {
                try {
                    const targets = JSON.parse(data);
                    const target = targets.find(t => t.url && t.url.includes(TARGET_SCRIPT));
                    if (target && target.webSocketDebuggerUrl) {
                        resolve(target.webSocketDebuggerUrl);
                    } else {
                        reject(new Error("No matching Node.js target found"));
                    }
                } catch (e) {
                    reject(e);
                }
            });
        }).on("error", reject);
    });
}

function applyPatch(wsUrl) {
    return new Promise((resolve, reject) => {
        const ws = new WebSocket(wsUrl);
        ws.on("open", () => {
            console.log("[PATCHER] Connected to V8. Enabling Debugger domain...");
            ws.send(JSON.stringify({ id: 1, method: "Debugger.enable" }));
        });

        ws.on("message", (data) => {
            const msg = JSON.parse(data.toString());
            if (msg.method === "Debugger.scriptParsed") {
                const url = msg.params.url;
                if (url && url.includes(TARGET_SCRIPT)) {
                    const scriptId = msg.params.scriptId;
                    console.log(`[PATCHER] Found Script ID ${scriptId}. Transmitting updated source...`);
                    
                    const sourceCode = fs.readFileSync(path.join(__dirname, TARGET_SCRIPT), "utf8");
                    ws.send(JSON.stringify({
                        id: 2,
                        method: "Debugger.setScriptSource",
                        params: { scriptId, scriptSource: sourceCode }
                    }));
                }
            }
            if (msg.id === 2) {
                if (msg.error) reject(new Error(msg.error.message || "setScriptSource failed"));
                else resolve();
                ws.close();
            }
        });
        ws.on("error", reject);
    });
}

async function main() {
    const pid = getStreamerPid();
    if (!pid) {
        console.error(`[PATCHER ERROR] Could not find running ${TARGET_SCRIPT} process.`);
        process.exit(1);
    }

    try {
        await enableInspector(pid);
        const wsUrl = await fetchDebuggerUrl();
        console.log(`[PATCHER] Discovered active WebSocket url: ${wsUrl}`);
        await applyPatch(wsUrl);
        console.log("[PATCHER SUCCESS] Streamer code updated live in V8 memory!");
        process.exit(0);
    } catch (err) {
        console.error("[PATCHER ERROR] Hotpatch failed:", err.message);
        process.exit(1);
    }
}

main();
