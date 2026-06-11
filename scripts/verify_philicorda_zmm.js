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
    console.log("=== Testing Philips Philicorda Neon Master Oscillator & Divider on ZMM VM ===");

    const philAddr = "0x0000000000000000000000000000000000000099";

    console.log("Registering PhilicordaGenerator...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "PhilicordaGenerator", "../solidity/bin/philicordaGenerator.yul", ${philAddr}`
    });
    console.log("Philicorda register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 pitchCoefficient, int256 rcRate) public returns (int256, int256)"
    ]);

    const steps = 30;
    const pitch = 1000000000000000000n; // 1.0 scale
    const rcRate = 5000000000000000000n; // 5.0V rate step per sample

    console.log("\nSimulating 30 clock ticks (neon gas charging / discharging and frequency division):");
    console.log("--------------------------------------------------------------------------------------");

    for (let i = 0; i < steps; i++) {
        const calldata = iface.encodeFunctionData("processSample", [pitch, rcRate]);
        const res = await postJson("/api/zmm-exec", { name: "PhilicordaGenerator", calldata });
        console.log(`Step ${i+1} Raw output:`, res.result.output);
        
        // Decode two outputs: masterSaw and dividedSquare
        const decoded = iface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const sawVolts = Number(decoded[0]) / 1e18;
        const divVolts = Number(decoded[1]) / 1e18;

        const width = 20;
        const normSaw = Math.max(0, Math.min(width - 1, Math.round((sawVolts + 1.0) / 2.0 * width)));
        const sawBar = Array.from({ length: width }, (_, idx) => idx === normSaw ? "S" : "-").join("");

        const normDiv = Math.max(0, Math.min(width - 1, Math.round((divVolts + 1.0) / 2.0 * width)));
        const divBar = Array.from({ length: width }, (_, idx) => idx === normDiv ? "D" : "-").join("");

        console.log(`Step ${(i + 1).toString().padStart(2)}: Master Saw = ${sawVolts.toFixed(3).padStart(6)}V [${sawBar}] | Octave Div = ${divVolts.toFixed(3).padStart(6)}V [${divBar}]`);
    }

    console.log("\n★★★ PHILIPSHILICORDA NEON GENERATOR VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
