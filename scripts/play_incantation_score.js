/**
 * Plays the Prophecy Incantation cascading note sequence directly into the shared CPU process.
 * This triggers note events that are pushed down to the active browser Vulkan Synth Dashboard.
 */

const http = require("http");

function playNoteREST(noteName, frequency, durationMs) {
    const payload = JSON.stringify({
        type: 'synth_poke',
        note: noteName,
        freq: frequency,
        control: 1 // Gate On
    });

    const sessionId = process.argv[2] || "global";
    const reqOptions = {
        hostname: "127.0.0.1",
        port: 3000,
        path: `/api/synth-feed?sessionId=${sessionId}`,
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "Content-Length": Buffer.byteLength(payload)
        }
    };

    // Send Gate-On
    const req = http.request(reqOptions, (res) => {
        res.on("data", () => {});
    });
    req.write(payload);
    req.end();

    // Send Gate-Off after duration
    setTimeout(() => {
        const offPayload = JSON.stringify({
            type: 'synth_poke',
            note: noteName,
            freq: frequency,
            control: 0 // Gate Off
        });

        const offReq = http.request({
            ...reqOptions,
            headers: {
                "Content-Type": "application/json",
                "Content-Length": Buffer.byteLength(offPayload)
            }
        }, (res) => {
            res.on("data", () => {});
        });
        offReq.write(offPayload);
        offReq.end();
    }, durationMs);
}

// Cascading score corresponding to the Prophecy Incantation (C4, E4, G4, B4, D5, A4)
const score = [
    { note: "C4", freq: 261.63, delay: 0 },
    { note: "E4", freq: 329.63, delay: 300 },
    { note: "G4", freq: 392.00, delay: 600 },
    { note: "B4", freq: 493.88, delay: 900 },
    { note: "D5", freq: 587.33, delay: 1200 },
    { note: "A4", freq: 440.00, delay: 1500 }
];

console.log("[CPU PLAYER] Broadcasting Prophecy Incantation note progression directly into CPU process feed...");
score.forEach(step => {
    setTimeout(() => {
        console.log(`[CPU PLAYER] Playing Note: ${step.note} (${step.freq} Hz)`);
        playNoteREST(step.note, step.freq, 220);
    }, step.delay);
});
