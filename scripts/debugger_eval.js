const WebSocket = require("ws");

const wsUrl = "ws://127.0.0.1:9229/1fb95398-5584-4a05-8ce7-78cdb3a7ad4b";
const ws = new WebSocket(wsUrl);

ws.on("open", () => {
    console.log("[EVAL] Connected to V8 debugger.");
    
    const evalCode = `
        (function() {
            try {
                if (global.wss) {
                    return "WebSocket server already running.";
                }
                const WebSocket = process.mainModule.require('ws');
                global.wss = new WebSocket.Server({ port: 9999 });

            global.wss.on("connection", (ws) => {
                console.log("[WEBSOCKET COAXIAL] Client connected to live broadcast control interface.");
                ws.on("message", (message) => {
                    try {
                        const data = JSON.parse(message.toString());
                        console.log("[WEBSOCKET COAXIAL] Received instruction packet:", data);
                        
                        if (data.tts_enabled !== undefined) global.coaxialState.tts_enabled = data.tts_enabled;
                        if (data.scroller) {
                            global.coaxialState.scroller = { ...global.coaxialState.scroller, ...data.scroller };
                            global.writeScrollerBinaryConfig(global.coaxialState.scroller);
                        }
                        if (data.audio) {
                            global.coaxialState.audio = { ...global.coaxialState.audio, ...data.audio };
                        }
                        if (data.camera) {
                            global.coaxialState.camera = { ...global.coaxialState.camera, ...data.camera };
                        }

                        if (data.draw_operations && global.lastCompleteFrame) {
                            for (const op of data.draw_operations) {
                                if (op.type === "draw_rect") {
                                    for (let dy = 0; dy < op.h; dy++) {
                                        for (let dx = 0; dx < op.w; dx++) {
                                            const px = op.x + dx;
                                            const py = op.y + dy;
                                            if (px >= 0 && px < 1280 && py >= 0 && py < 692) {
                                                const idx = (py * 1280 + px) * 3;
                                                global.lastCompleteFrame[idx] = op.r;
                                                global.lastCompleteFrame[idx + 1] = op.g;
                                                global.lastCompleteFrame[idx + 2] = op.b;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        ws.send(JSON.stringify({ status: "success", info: "Coaxial instruction packet applied live." }));
                    } catch (e) {
                        ws.send(JSON.stringify({ status: "error", info: e.message }));
                    }
                });
            });
            return "WebSocket server started successfully on port 9999.";
            } catch (e) {
                return "ERROR: " + e.message + "\\n" + e.stack;
            }
        })();
    `;
    
    ws.send(JSON.stringify({
        id: 1,
        method: "Runtime.evaluate",
        params: {
            expression: evalCode
        }
    }));
});

ws.on("message", (data) => {
    const msg = JSON.parse(data.toString());
    if (msg.id === 1) {
        if (msg.error) {
            console.error("[EVAL ERROR]:", msg.error);
        } else {
            console.log("[EVAL FULL MSG]:", JSON.stringify(msg, null, 2));
        }
        ws.close();
        process.exit(msg.error ? 1 : 0);
    }
});
