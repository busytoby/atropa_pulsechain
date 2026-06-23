const assert = require("assert");
const http = require("http");

console.log("=============================================================");
console.log("Auncient ZMM VM cpu6502 Emulator Unit Test Suite");
console.log("=============================================================");

// Helper to make HTTP POST requests to the ZMM VM backend
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

// Helper to execute a method on the cpu6502 contract
async function callCPU(selector, calldata = "") {
    const payload = {
        name: "cpu6502",
        calldata: selector + calldata
    };
    const res = await postJSON("/api/zmm-exec", payload);
    return res.result;
}

async function runTests() {
    try {
        console.log("1. Initializing cpu6502 contract via YULINIT in ZMM VM...");
        const initRes = await postJSON("/api/zmm-exec", {
            code: 'YULINIT "cpu6502", "../solidity/bin/cpu6502.yul", 1'
        });
        assert.ok(initRes, "YULINIT should succeed");
        console.log("   ✓ cpu6502 contract successfully initialized.");

        console.log("2. Fetching initial CPU registers via getCPUState...");
        // Selector 0xe0f77c57 for getCPUState()
        const stateRes = await callCPU("e0f77c57");
        console.log("DEBUG stateRes:", JSON.stringify(stateRes, null, 2));
        assert.ok(stateRes && stateRes.output, "Should return CPU registers");
        
        const parseRegs = (hex) => {
            const chunks = [];
            for (let i = 0; i < 6; i++) {
                chunks.push(hex.substring(i * 64, (i + 1) * 64));
            }
            return chunks;
        };

        const regs = parseRegs(stateRes.output.trim());
        console.log("   ✓ Initial registers (Hex):", regs);

        console.log("3. Executing LDA Immediate opcode ($A9) with operand $42 via executeOp...");
        // Selector 0x2a9cd5d1 for executeOp(uint8 opcode, uint256 operand)
        // opcode A9 -> padded to 32 bytes, operand 42 -> padded to 32 bytes
        const opcodeCalldata = "a9".padEnd(64, "0");
        const operandCalldata = "42".padStart(64, "0");
        
        const execRes = await callCPU("2a9cd5d1", opcodeCalldata + operandCalldata);
        assert.ok(execRes, "executeOp should return successfully");
        
        console.log("4. Fetching updated CPU registers to verify Accumulator A value...");
        const updatedStateRes = await callCPU("e0f77c57");
        const updatedRegs = parseRegs(updatedStateRes.output.trim());
        console.log("   ✓ Updated registers (Hex):", updatedRegs);
        
        // Accumulator A is the first register returned by getCPUState (index 0)
        const accA = parseInt(updatedRegs[0], 16);
        assert.strictEqual(accA, 0x42, "Accumulator A must hold the loaded value $42");
        console.log("   ✓ Accumulator A successfully loaded with $42.");

        console.log("\n=============================================================");
        console.log("AUNCIENT CPU6502 EMULATOR UNIT TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err.message);
        process.exit(1);
    }
}

runTests();
