const http = require("http");

const payload = JSON.stringify({
    type: 'synth_reset'
});

const req = http.request({
    hostname: "127.0.0.1",
    port: 3000,
    path: "/api/synth-feed?sessionId=global",
    method: "POST",
    headers: {
        "Content-Type": "application/json",
        "Content-Length": Buffer.byteLength(payload)
    }
}, (res) => {
    res.on("data", () => {});
    res.on("end", () => {
        console.log("[RESET] Synth reset incantation successfully sent to dashboard.");
    });
});

req.on("error", (e) => {
    console.error(`Error sending reset: ${e.message}`);
});

req.write(payload);
req.end();
