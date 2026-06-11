const http = require("http");
const { ethers } = require("ethers");

function postJson(path, payload) {
    return new Promise((resolve, reject) => {
        const dataStr = JSON.stringify(payload);
        const req = http.request({
            hostname: "127.0.0.1",
            port: 3000,
            path: path,
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": dataStr.length
            }
        }, (res) => {
            let body = "";
            res.on("data", chunk => body += chunk);
            res.on("end", () => {
                try {
                    resolve(JSON.parse(body));
                } catch (e) {
                    reject(new Error(`Failed to parse response: ${body}`));
                }
            });
        });
        req.on("error", reject);
        req.write(dataStr);
        req.end();
    });
}

async function test() {
    console.log("=== Testing Wien-Bridge Selective Tone Decoder on ZMM VM ===");

    const semAddress = "0x0000000000000000000000000000000000000077";

    console.log("Registering SemafoonDecoder...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "SemafoonDecoder", "../solidity/bin/semafoonDecoder.yul", ${semAddress}`
    });
    console.log("Semafoon registration response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 inputSignal, int256 rcDecay) public returns (int256, int256)"
    ]);

    const steps = 15;
    const decay = 200000000000000000n; // 0.2 envelope charging rate

    // Scenario A: Resonant Frequency (Tuned input)
    console.log("\n--- Scenario A: Input at Resonant Frequency -> Envelope Ascends ---");
    for (let i = 0; i < steps; i++) {
        // High-frequency target oscillation (approximating resonance)
        const angle = i * 1.2; 
        const inputVolts = Math.sin(angle) * 1.0;
        const scaledInput = BigInt(Math.round(inputVolts * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledInput, decay]);
        const res = await postJson("/api/zmm-exec", { name: "SemafoonDecoder", calldata });
        
        const decoded = iface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const filtered = Number(decoded[0]) / 1e18;
        const envelope = Number(decoded[1]) / 1e18;

        const width = 20;
        const norm = Math.max(0, Math.min(width - 1, Math.round(envelope * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx <= norm ? "█" : "-").join("");

        console.log(`Step ${(i + 1).toString().padStart(2)}: Filtered Out = ${filtered.toFixed(3).padStart(6)}V | Env = ${envelope.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    // Reset decoder storage for Scenario B
    console.log("\nRe-registering to flush filter state...");
    await postJson("/api/zmm-exec", {
        code: `YULINIT "SemafoonDecoder", "../solidity/bin/semafoonDecoder.yul", ${semAddress}`
    });

    // Scenario B: Non-Resonant Frequency (Off-target input)
    console.log("\n--- Scenario B: Input at Off-Target Frequency -> Envelope Blocked ---");
    for (let i = 0; i < steps; i++) {
        // Very low frequency oscillation
        const angle = i * 0.1; 
        const inputVolts = Math.sin(angle) * 1.0;
        const scaledInput = BigInt(Math.round(inputVolts * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledInput, decay]);
        const res = await postJson("/api/zmm-exec", { name: "SemafoonDecoder", calldata });
        
        const decoded = iface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const filtered = Number(decoded[0]) / 1e18;
        const envelope = Number(decoded[1]) / 1e18;

        const width = 20;
        const norm = Math.max(0, Math.min(width - 1, Math.round(envelope * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx <= norm ? "█" : "-").join("");

        console.log(`Step ${(i + 1).toString().padStart(2)}: Filtered Out = ${filtered.toFixed(3).padStart(6)}V | Env = ${envelope.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    console.log("\n★★★ SEMAFOON Wien-Bridge SELECTIVE TONE DECODER VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
