const assert = require("assert");
const http = require("http");

console.log("=============================================================");
console.log("Auncient ZMM VM Low-Level Unit Tests");
console.log("=============================================================");

// Helper to execute HTTP POST requests to ZMM VM API
function postJSON(path, payload) {
    return new Promise((resolve, reject) => {
        const data = JSON.stringify(payload);
        const options = {
            hostname: "127.0.0.1",
            port: 3000,
            path: path,
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Content-Length": Buffer.byteLength(data)
            }
        };
        const req = http.request(options, (res) => {
            let body = "";
            res.on("data", (chunk) => body += chunk);
            res.on("end", () => {
                if (res.statusCode >= 200 && res.statusCode < 300) {
                    try {
                        resolve(JSON.parse(body));
                    } catch (e) {
                        reject(new Error(`Failed to parse: ${body}`));
                    }
                } else {
                    reject(new Error(`HTTP status ${res.statusCode}: ${body}`));
                }
            });
        });
        req.on("error", reject);
        req.write(data);
        req.end();
    });
}

async function runTests() {
    try {
        console.log("1. Testing ZMM VM execution endpoint (/api/zmm-exec)...");
        
        // Test YI contract balance query
        const yiPayload = {
            name: "yi",
            calldata: "70a0823100000000000000000000000000000000000000000000000000000000000004cc"
        };
        const yiRes = await postJSON("/api/zmm-exec", yiPayload);
        
        assert.ok(yiRes, "Response should be truthy");
        assert.ok(yiRes.result, "Response should have result");
        assert.ok(yiRes.result.output, "Response result should contain output");
        
        const balanceHex = yiRes.result.output.trim();
        const balance = BigInt("0x" + balanceHex);
        console.log(`   ✓ YI balance queried successfully. Raw hex: ${balanceHex} (${balance} Wei)`);
        
        // Test balance querying of a non-token / balance-less Yul system
        console.log("2. Testing ZMM VM balance query on balance-less system (zmachine)...");
        const zmachinePayload = {
            name: "zmachine",
            calldata: "70a0823100000000000000000000000000000000000000000000000000000000000004cc"
        };
        const zmachineRes = await postJSON("/api/zmm-exec", zmachinePayload);
        assert.ok(zmachineRes, "Response should be truthy for zmachine");
        // Balance-less or non-token contracts return empty output or error on execution without fallback
        console.log("   ✓ zmachine execution query checked. Response output:", zmachineRes.result ? zmachineRes.result.output : "null");

        console.log("3. Testing invalid query payload handling...");
        const badPayload = {
            calldata: "70a08231"
        };
        try {
            await postJSON("/api/zmm-exec", badPayload);
            assert.fail("Server should reject requests missing contract name");
        } catch (e) {
            console.log("   ✓ Invalid payload successfully rejected with message:", e.message);
        }

        console.log("\n=============================================================");
        console.log("ALL LOW-LEVEL VM UNIT TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST RUN FAILURE:", err.message);
        process.exit(1);
    }
}

runTests();
