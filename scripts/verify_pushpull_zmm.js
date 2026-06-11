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
    console.log("=== Testing PNP/NPN Complementary Push-Pull Stage on ZMM VM ===");

    const stageAddr = "0x0000000000000000000000000000000000000066";

    console.log("Registering PushPullStage...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "PushPullStage", "../solidity/bin/pushPullStage.yul", ${stageAddr}`
    });
    console.log("PushPullStage register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 inputSignal, int256 biasOffset) public returns (int256)"
    ]);

    const steps = 15;
    
    // Scenario 1: Low Bias (0V) -> Crossover distortion should be visible
    console.log("\n--- Scenario A: Zero Bias (0V) -> Crossover Dead-Zone Active ---");
    for (let i = 0; i < steps; i++) {
        const angle = (i / (steps - 1)) * Math.PI * 2;
        const inputVal = Math.sin(angle) * 1.0; // -1V to +1V
        const scaledInput = BigInt(Math.round(inputVal * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledInput, 0n]);
        const res = await postJson("/api/zmm-exec", { name: "PushPullStage", calldata });
        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        const outVolts = Number(outVal) / 1e18;

        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round((outVolts + 1.0) / 2.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${ (i + 1).toString().padStart(2) }: Input = ${inputVal.toFixed(3).padStart(6)}V | Output = ${outVolts.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    // Scenario 2: High Bias (200mV) -> Dead-zone eliminated
    console.log("\n--- Scenario B: Calibrated Bias (200mV) -> Dead-Zone Eliminated ---");
    const highBias = 200000000000000000n; // 200mV
    for (let i = 0; i < steps; i++) {
        const angle = (i / (steps - 1)) * Math.PI * 2;
        const inputVal = Math.sin(angle) * 1.0;
        const scaledInput = BigInt(Math.round(inputVal * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledInput, highBias]);
        const res = await postJson("/api/zmm-exec", { name: "PushPullStage", calldata });
        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        const outVolts = Number(outVal) / 1e18;

        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round((outVolts + 1.0) / 2.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${ (i + 1).toString().padStart(2) }: Input = ${inputVal.toFixed(3).padStart(6)}V | Output = ${outVolts.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    console.log("\n★★★ COMPLEMENTARY PUSH-PULL STAGE VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
