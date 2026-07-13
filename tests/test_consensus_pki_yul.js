const assert = require("assert");
const http = require("http");

console.log("=============================================================");
console.log("Auncient ZMM VM ConsensusPKI Contract Integration Test Suite");
console.log("=============================================================");

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

async function callContract(selector, calldata = "") {
    const payload = {
        name: "ConsensusPKI",
        calldata: selector + calldata
    };
    const res = await postJSON("/api/zmm-exec", payload);
    return res.result;
}

async function runTests() {
    try {
        console.log("1. Initializing ConsensusPKI contract via YULINIT in ZMM VM...");
        const initRes = await postJSON("/api/zmm-exec", {
            code: 'YULINIT "ConsensusPKI", "../solidity/bin/ConsensusPKI.yul", 1'
        });
        assert.ok(initRes, "YULINIT should succeed");
        console.log("   ✓ ConsensusPKI contract successfully initialized.");

        console.log("2. Setting Validator #5 key and stake...");
        // selector for setValidator(uint256,address,uint256) -> 0xd88a05c3
        // index 5 -> padded
        const indexCalldata = "5".padStart(64, "0");
        // address 0x90F79bf6EB2c4f870365E785982E1f101E93b906 -> padded
        const addrCalldata = "90F79bf6EB2c4f870365E785982E1f101E93b906".padStart(64, "0");
        // stake 32 -> padded
        const stakeCalldata = "20".padStart(64, "0"); // 0x20 = 32

        const setRes = await callContract("d88a05c3", indexCalldata + addrCalldata + stakeCalldata);
        assert.ok(setRes, "setValidator should execute successfully");
        console.log("   ✓ Validator #5 successfully registered in ZMM VM storage.");

        console.log("3. Querying Validator #5 stake to verify...");
        // selector for getValidatorStake(uint256) -> 0x3d02a013
        const getRes = await callContract("3d02a013", indexCalldata);
        console.log("DEBUG getRes:", JSON.stringify(getRes, null, 2));
        assert.ok(getRes && getRes.output, "Should return stake");
        
        const retrievedStake = parseInt(getRes.output.trim(), 16);
        assert.strictEqual(retrievedStake, 32, "Initial stake must be 32 PLS");
        console.log("   ✓ Initial stake verification passed.");

        console.log("\n=============================================================");
        console.log("AUNCIENT CONSENSUS PKI INTEGRATION TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err.message);
        process.exit(1);
    }
}

runTests();
