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
    console.log("=== Testing Germanium Common-Emitter Transistor Stage on ZMM VM ===");

    const gerAddress = "0x0000000000000000000000000000000000000071";

    console.log("Registering GermaniumStage...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "GermaniumStage", "../solidity/bin/germaniumStage.yul", ${gerAddress}`
    });
    console.log("Germanium stage registration response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 inputVoltage) public returns (int256)"
    ]);

    const steps = 15;
    console.log("\nSimulating 15 sample steps driving the Germanium transistor stage with a sine wave input:");
    console.log("-----------------------------------------------------------------------------------------");

    for (let i = 0; i < steps; i++) {
        // Sine wave swing from -0.8V to +0.8V
        const angle = (i / (steps - 1)) * Math.PI * 2;
        const inputVolts = Math.sin(angle) * 0.8;
        const scaledInput = BigInt(Math.round(inputVolts * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledInput]);
        const res = await postJson("/api/zmm-exec", { name: "GermaniumStage", calldata });
        
        const decoded = iface.decodeFunctionResult("processSample", "0x" + res.result.output);
        const outVolts = Number(decoded[0]) / 1e18;

        const width = 30;
        // Map output volts (which swings up to VCC=9V, typical range 1V to 9V)
        const norm = Math.max(0, Math.min(width - 1, Math.round((outVolts - 1.0) / 8.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${(i + 1).toString().padStart(2)}: Input = ${inputVolts.toFixed(3).padStart(6)}V | Output = ${outVolts.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    console.log("\n★★★ GERMANIUM COMMON-EMITTER STAGE VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
