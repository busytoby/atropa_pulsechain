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
    console.log("=== Testing TransducerStage on ZMM VM ===");

    const transducerAddr = "0x0000000000000000000000000000000000000077";

    console.log("Registering TransducerStage...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "TransducerStage", "../solidity/bin/transducerStage.yul", ${transducerAddr}`
    });
    console.log("TransducerStage register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 pressure, int256 bias) public returns (int256)"
    ]);

    // Constant electrical bias of 700mV (0.7V = 7e17)
    const biasInput = 700000000000000000n;

    // Acoustic pressure wave (sine wave ranging from -5V to +5V)
    const steps = 20;
    console.log("\nProcessing dynamic sound pressure wave through Raytheon Transducer Transistor:");
    for (let i = 0; i < steps; i++) {
        const angle = (i / (steps - 1)) * Math.PI * 2;
        const pressureVal = Math.sin(angle) * 5.0; // -5 to +5 amplitude
        const scaledPressure = BigInt(Math.round(pressureVal * 1e18));

        const calldata = iface.encodeFunctionData("processSample", [scaledPressure, biasInput]);
        const res = await postJson("/api/zmm-exec", { name: "TransducerStage", calldata });
        
        if (res.error) {
            console.error(`Step ${i + 1} Error:`, res.error);
            continue;
        }

        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        const outVolts = Number(outVal) / 1e18;

        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round(outVolts / 9.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx === norm ? "O" : "-").join("");

        console.log(`Step ${ (i + 1).toString().padStart(2) }: Pressure = ${pressureVal.toFixed(3).padStart(6)}V | Collector Out = ${outVolts.toFixed(3).padStart(6)}V | [${bar}]`);
    }

    console.log("\n★★★ TRANSDUCER STAGE VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
