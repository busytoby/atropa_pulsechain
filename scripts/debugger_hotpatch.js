const fs = require("fs");
const path = require("path");
const http = require("http");
const WebSocket = require("ws");

// Dynamically fetch active inspector UUID from localhost JSON endpoint
http.get("http://127.0.0.1:9229/json", (res) => {
    let rawData = "";
    res.on("data", (chunk) => { rawData += chunk; });
    res.on("end", () => {
        try {
            const targets = JSON.parse(rawData);
            const target = targets.find(t => t.url && t.url.includes("tsfi_live_streamer.js"));
            if (!target || !target.webSocketDebuggerUrl) {
                console.error("[HOTPATCH ERROR] Could not find debugger target for tsfi_live_streamer.js");
                process.exit(1);
            }
            
            const wsUrl = target.webSocketDebuggerUrl;
            console.log(`[HOTPATCH] Connecting to dynamic wsUrl: ${wsUrl}`);
            connectDebugger(wsUrl);
        } catch (e) {
            console.error("[HOTPATCH ERROR] Failed parsing targets:", e.message);
            process.exit(1);
        }
    });
}).on("error", (err) => {
    console.error("[HOTPATCH ERROR] Failed to query debugger endpoint:", err.message);
    process.exit(1);
});

function connectDebugger(wsUrl) {
    const ws = new WebSocket(wsUrl);

    ws.on("open", () => {
        console.log("[HOTPATCH] Connected to V8 debugger.");
        ws.send(JSON.stringify({ id: 1, method: "Debugger.enable" }));
    });

    ws.on("message", (data) => {
        const msg = JSON.parse(data.toString());
        
        if (msg.method === "Debugger.scriptParsed") {
            const url = msg.params.url;
            if (url && url.includes("tsfi_live_streamer.js")) {
                const scriptId = msg.params.scriptId;
                console.log(`[HOTPATCH] Discovered script ID: ${scriptId} for ${url}`);
                
                const sourceCode = fs.readFileSync(path.join(__dirname, "tsfi_live_streamer.js"), "utf8");
                
                const setSourceMsg = {
                    id: 2,
                    method: "Debugger.setScriptSource",
                    params: {
                        scriptId: scriptId,
                        scriptSource: sourceCode
                    }
                };
                ws.send(JSON.stringify(setSourceMsg));
            }
        }
        
        if (msg.id === 2) {
            if (msg.error) {
                console.error("[HOTPATCH ERROR] Failed to set script source:", msg.error);
            } else {
                console.log("[HOTPATCH SUCCESS] Hot-patched running streamer source code live with zero downtime!");
            }
            ws.close();
            process.exit(msg.error ? 1 : 0);
        }
    });
}
