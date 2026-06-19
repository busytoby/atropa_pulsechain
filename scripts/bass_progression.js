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

// Deep bass line: E2 (82.41 Hz), G2 (98.00 Hz), A2 (110.00 Hz), C3 (130.81 Hz), B2 (123.47 Hz)
const score = [
    { note: "E2", freq: 82.41, delay: 0 },
    { note: "G2", freq: 98.00, delay: 800 },
    { note: "A2", freq: 110.00, delay: 1600 },
    { note: "C3", freq: 130.81, delay: 2400 },
    { note: "B2", freq: 123.47, delay: 3200 },
    { note: "E2", freq: 82.41, delay: 4000 }
];

console.log("[CPU PLAYER] Broadcasting heavy bass progression to the Vulkan display...");
score.forEach(step => {
    setTimeout(() => {
        console.log(`[CPU PLAYER] Playing Bass: ${step.note} (${step.freq} Hz)`);
        playNoteREST(step.note, step.freq, 650);
    }, step.delay);
});
