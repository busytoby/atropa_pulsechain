const fs = require("fs");
const path = require("path");
const http = require("http");

const JCL_PATH = path.join(__dirname, "../solidity/dysnomia/domain/jcl/playsynth.jcl");

const NOTE_FREQS = {
    "C4": 261.63,
    "D4": 293.66,
    "E4": 329.63,
    "F4": 349.23,
    "G4": 392.00,
    "A4": 440.00,
    "B4": 493.88
};

function postSynthNote(freq, controlVal) {
    const payload = JSON.stringify({
        type: 'synth_poke',
        note: 'JCL_LIVE',
        freq: freq,
        control: controlVal
    });

    const reqOptions = {
        hostname: "127.0.0.1",
        port: 3000,
        path: "/api/synth-feed?sessionId=global",
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "Content-Length": Buffer.byteLength(payload)
        }
    };

    const req = http.request(reqOptions, (res) => {
        res.on("data", () => {});
    });
    req.on("error", (e) => {
        console.error("[LIVE PLAY] Connection error to synth-feed API:", e.message);
    });
    req.write(payload);
    req.end();
}

async function run() {
    console.log("[LIVE PLAY] Parsing playsynth.jcl note cards...");
    if (!fs.existsSync(JCL_PATH)) {
        console.error("[LIVE PLAY] Error: playsynth.jcl not found at", JCL_PATH);
        return;
    }

    const content = fs.readFileSync(JCL_PATH, "utf8");
    const lines = content.split("\n");

    for (const line of lines) {
        const trimmed = line.trim();
        if (trimmed.startsWith("SYNTH NOTE")) {
            const parts = trimmed.split(/\s+/);
            // Expected format: SYNTH NOTE C4 LEVEL 8
            if (parts.length >= 4) {
                const noteName = parts[2];
                const freq = NOTE_FREQS[noteName];
                if (freq) {
                    console.log(`[LIVE PLAY] Playing Pitch: ${noteName} (${freq} Hz)`);
                    postSynthNote(freq, 1); // Note On
                    await new Promise(resolve => setTimeout(resolve, 500));
                    postSynthNote(freq, 0); // Note Off
                    await new Promise(resolve => setTimeout(resolve, 100));
                }
            }
        }
    }
    console.log("[LIVE PLAY] JCL play scale execution completed.");
}

run();
