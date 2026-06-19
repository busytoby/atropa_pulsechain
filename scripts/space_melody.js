const http = require("http");

function playNoteREST(noteName, frequency, durationMs) {
    const payload = JSON.stringify({
        type: 'synth_poke',
        note: noteName,
        freq: frequency,
        control: 1 // Gate On
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
    req.write(payload);
    req.end();

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

// A new spacey minor progression: A4, C5, F5, E5, D5, B4, C5, A4
const score = [
    { note: "A4", freq: 440.00, delay: 0 },
    { note: "C5", freq: 523.25, delay: 600 },
    { note: "F5", freq: 698.46, delay: 1200 },
    { note: "E5", freq: 659.25, delay: 1800 },
    { note: "D5", freq: 587.33, delay: 2400 },
    { note: "B4", freq: 493.88, delay: 3000 },
    { note: "C5", freq: 523.25, delay: 3600 },
    { note: "A4", freq: 440.00, delay: 4200 }
];

console.log("[CPU PLAYER] Broadcasting a new Auncient spacey melody to the Vulkan display...");
score.forEach(step => {
    setTimeout(() => {
        console.log(`[CPU PLAYER] Playing: ${step.note} (${step.freq} Hz)`);
        playNoteREST(step.note, step.freq, 450);
    }, step.delay);
});
