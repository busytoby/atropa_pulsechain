const WebSocket = require("ws");

const ws = new WebSocket("ws://127.0.0.1:9999");

ws.on("open", () => {
    console.log("[CLIENT] Connected to Coaxial WebSocket Server on port 9999.");
    
    // Command payload modifying audio, scroller parameters, and writing raw rectangle overlay on the framebuffer
    const payload = {
        tts_enabled: false,
        scroller: {
            text: "TSFi/2",
            color_scheme: 4,
            speed_scale: 1.2,
            bear_count: 5
        },
        audio: {
            tempo_scale: 0.8,
            transpose_offset: -3,
            persistent_theme: true
        },
        draw_operations: [
            // Draw a bright purple progress/status indicator bar directly in the live framebuffer
            {
                type: "draw_rect",
                x: 60,
                y: 600,
                w: 1160,
                h: 20,
                r: 128,
                g: 0,
                b: 128
            }
        ]
    };
    
    ws.send(JSON.stringify(payload));
});

ws.on("message", (data) => {
    console.log("[CLIENT] Server response:", data.toString());
    ws.close();
    process.exit(0);
});

ws.on("error", (err) => {
    console.error("[CLIENT ERROR]:", err.message);
    process.exit(1);
});
