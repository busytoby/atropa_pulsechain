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
    console.log("=== Testing EnvelopeDetector on ZMM VM ===");

    const envelopeAddr = "0x0000000000000000000000000000000000000099";

    console.log("Registering EnvelopeDetector...");
    const initDb = await postJson("/api/zmm-exec", {
        code: `YULINIT "EnvelopeDetector", "../solidity/bin/envelopeDetector.yul", ${envelopeAddr}`
    });
    console.log("EnvelopeDetector register response:", JSON.stringify(initDb));

    const iface = new ethers.Interface([
        "function processSample(int256 inputVoltage) public returns (int256)"
    ]);

    // Feed a series of input voltages using BigInt to prevent JavaScript precision overflow
    const inputs = [
        0n,                  // 0V
        100000000000000000n, // 100mV (below conduction offset of 150mV)
        500000000000000000n, // 500mV (should start conducting base-emitter/diode)
        2000000000000000000n, // 2V spike
        2000000000000000000n, // 2V spike continued
        0n,                  // Input drops to 0 (envelope should slowly decay)
        0n,
        0n,
        0n,
        0n,
        0n,
        0n,
        0n,
        0n,
        0n
    ];

    console.log("\nProcessing inputs through OA85 Germanium Diode Envelope Tracker:");
    for (let i = 0; i < inputs.length; i++) {
        const calldata = iface.encodeFunctionData("processSample", [inputs[i]]);
        const res = await postJson("/api/zmm-exec", { name: "EnvelopeDetector", calldata });
        
        if (res.error) {
            console.error(`Step ${i + 1} Error:`, res.error);
            continue;
        }

        const outVal = iface.decodeFunctionResult("processSample", "0x" + res.result.output)[0];
        
        const inVolts = Number(inputs[i]) / 1e18;
        const outVolts = Number(outVal) / 1e18;

        const width = 30;
        const norm = Math.max(0, Math.min(width - 1, Math.round(outVolts / 2.0 * width)));
        const bar = Array.from({ length: width }, (_, idx) => idx <= norm ? "#" : "-").join("");

        console.log(`Step ${ (i + 1).toString().padStart(2) }: Input = ${inVolts.toFixed(3).padStart(6)}V | Env Out = ${outVolts.toFixed(6).padStart(8)}V | [${bar}]`);
    }

    console.log("\n★★★ ENVELOPE DETECTOR VERIFICATION COMPLETED ★★★");
}

test().catch(console.error);
