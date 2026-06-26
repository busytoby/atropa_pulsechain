const fs = require("fs");
const http = require("http");
const path = require("path");

const REGISTRY_FILE = path.join(__dirname, "..", "active_performance_registry.bin");
const FRAME_SIZE = 4408;
const COLLISION_OFFSET = 296;

let lastReadOffset = 0;
let currentScaleOffset = 0; // Transposition offset

// Bass scales to cycle through on X-axis collision (Bit 0)
const SCALES = [
    { root: 82.41, name: "E2" },  // E2 baseline
    { root: 98.00, name: "G2" },  // G2 transposition
    { root: 110.00, name: "A2" }, // A2 transposition
    { root: 123.47, name: "B2" }  // B2 transposition
];

function playSynthNote(freq, controlVal, durationMs) {
    const payload = JSON.stringify({
        type: 'synth_poke',
        note: 'COLLISION',
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
        // Suppress connection errors if server is not up
    });
    req.write(payload);
    req.end();

    if (controlVal === 1 && durationMs > 0) {
        setTimeout(() => {
            playSynthNote(freq, 0, 0); // Gate off
        }, durationMs);
    }
}

function processFrame(buffer) {
    const flags = buffer.readUInt32LE(COLLISION_OFFSET);
    if (flags === 0) return;

    console.log(`[BRIDGE] Detected collision flags: ${flags}`);

    // Bit 0: X-axis collision -> Transpose note scale
    if (flags & 1) {
        currentScaleOffset = (currentScaleOffset + 1) % SCALES.length;
        const newScale = SCALES[currentScaleOffset];
        console.log(`[BRIDGE] X-Axis Bounce! Transposing baseline root scale to ${newScale.name} (${newScale.root} Hz)`);
        // Play note to confirm transposition
        playSynthNote(newScale.root, 1, 300);
    }

    // Bit 1: Y-axis collision -> Trigger percussion drum impact
    if (flags & 2) {
        console.log(`[BRIDGE] Y-Axis Impact! Triggering synthetic drum note...`);
        // Play drum impact frequency (150 Hz)
        playSynthNote(150.0, 1, 150);
    }

    // Bit 2: Z-axis collision -> Modulate cutoff frequency
    if (flags & 4) {
        console.log(`[BRIDGE] Z-Axis Collision! Modulating filter resonance sweeps...`);
        // Rapid pitch sweep/chirp
        playSynthNote(800.0, 1, 100);
    }
}

function pollRegistry() {
    if (!fs.existsSync(REGISTRY_FILE)) {
        setTimeout(pollRegistry, 100);
        return;
    }

    try {
        const stats = fs.statSync(REGISTRY_FILE);
        if (stats.size > lastReadOffset) {
            const fd = fs.openSync(REGISTRY_FILE, "r");
            const newBytes = stats.size - lastReadOffset;
            const buffer = Buffer.alloc(newBytes);
            fs.readSync(fd, buffer, 0, newBytes, lastReadOffset);
            fs.closeSync(fd);

            lastReadOffset = stats.size;

            // Process all newly written full frames
            for (let i = 0; i + FRAME_SIZE <= buffer.length; i += FRAME_SIZE) {
                const frameBuf = buffer.slice(i, i + FRAME_SIZE);
                processFrame(frameBuf);
            }
        }
    } catch (e) {
        console.error("[BRIDGE] Read error:", e);
    }

    setTimeout(pollRegistry, 50);
}

// Reset log offset if file is recreated
fs.watch(path.dirname(REGISTRY_FILE), (eventType, filename) => {
    if (filename === path.basename(REGISTRY_FILE)) {
        if (!fs.existsSync(REGISTRY_FILE)) {
            lastReadOffset = 0;
        }
    }
});

console.log("[BRIDGE] Starting physics-to-audio WinchesterMQ bridge...");
pollRegistry();
