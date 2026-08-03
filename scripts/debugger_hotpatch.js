const fs = require("fs");
const path = require("path");
const WebSocket = require("ws");

const wsUrl = "ws://127.0.0.1:9229/9e37988c-896c-48f2-817b-51f6b88469ea";
const ws = new WebSocket(wsUrl);

ws.on("open", () => {
    console.log("[HOTPATCH] Connected to V8 debugger.");
    
    // Enable Debugger domain
    ws.send(JSON.stringify({ id: 1, method: "Debugger.enable" }));
});

ws.on("message", (data) => {
    const msg = JSON.parse(data.toString());
    
    // Catch scriptParsed events to find the target script ID
    if (msg.method === "Debugger.scriptParsed") {
        const url = msg.params.url;
        if (url && url.includes("tsfi_live_streamer.js")) {
            const scriptId = msg.params.scriptId;
            console.log(`[HOTPATCH] Discovered script ID: ${scriptId} for ${url}`);
            
            // Read new source code from disk
            const sourceCode = fs.readFileSync(path.join(__dirname, "tsfi_live_streamer.js"), "utf8");
            
            // Hot-patch the running script source!
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
