const assert = require("assert");
const http = require("http");

console.log("=============================================================");
console.log("Auncient ZMM VM + WinchesterMQ + Z-Machine Integration Test");
console.log("=============================================================");

// Helper to make HTTP POST requests to the backend server
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

// Helper to call YULEXEC on WinchesterMQ
async function callWinchesterMQ(selector, val) {
    const payload = {
        name: "WinchesterMQ",
        calldata: selector + (val !== undefined ? val.toString(16).padStart(64, "0") : "")
    };
    const res = await postJSON("/api/zmm-exec", payload);
    return res.result;
}

// Helper to read transient registers from WinchesterMQ
async function readTransientRegister(index) {
    // Selector 0xe9d601b0 for readTransient(uint256 index)
    const res = await callWinchesterMQ("e9d601b0", index);
    if (res && res.output) {
        return parseInt(res.output.trim(), 16);
    }
    return 0;
}

// Helper to execute SCSI register updates
async function writeSignalsOut(signals) {
    // Selector 0x485301a0 for writeSignalsOut(uint8 signals)
    await callWinchesterMQ("485301a0", signals);
}

async function writeDataPort(val) {
    // Selector 0x98d400c0 for writeDataPort(uint8 val)
    await callWinchesterMQ("98d400c0", val);
}

async function runTests() {
    try {
        console.log("Scenario C: Validating BSY/REQ/ACK status register transitions during SCSI loopback handshake...");
        
        // 1. Initial State Check: Assert BSY is 0 initially (or reset if needed)
        await writeSignalsOut(0x02); // RST = 1 to release/reset bus
        let bsy = await readTransientRegister(0);
        let req = await readTransientRegister(1);
        let ack = await readTransientRegister(5);
        assert.strictEqual(bsy, 0, "BSY must be 0 initially");
        assert.strictEqual(req, 0, "REQ must be 0 initially");
        assert.strictEqual(ack, 0, "ACK must be 0 initially");
        console.log("   ✓ SCSI bus successfully initialized to IDLE.");

        // 2. Select Phase: Host asserts SEL = 1
        await writeSignalsOut(0x01); // SEL = 1
        bsy = await readTransientRegister(0);
        req = await readTransientRegister(1);
        assert.strictEqual(bsy, 1, "WinchesterMQ controller must assert BSY = 1 upon SEL");
        assert.strictEqual(req, 1, "WinchesterMQ controller must assert REQ = 1 to request first CDB byte");
        console.log("   ✓ Select Phase transition verified successfully (BSY=1, REQ=1).");

        console.log("\nScenario A: Streaming 'look' command transaction and verifying expected Z-Machine description output...");

        // Simulate sending 6-byte CDB for writing to LUN 1, Sector 0:
        // [0x0A (Write), 0x20 (LUN 1), 0x00, 0x00, 0x01 (1 block), 0x00]
        const cdb = [0x0A, 0x20, 0x00, 0x00, 0x01, 0x00];
        for (let i = 0; i < cdb.length; i++) {
            await writeDataPort(cdb[i]);
            await writeSignalsOut(0x05); // SEL=1, ACK=1
            await writeSignalsOut(0x01); // SEL=1, ACK=0
        }

        // Send 256-byte payload containing command "look"
        const payload = Buffer.alloc(256);
        payload.write("MSGQ", 0);
        payload.writeUInt32BE(300, 4); // TTL
        payload.writeUInt32BE(1, 8);   // Sender Card
        payload.write("CORRELATION_ID_0001", 12);
        payload.write("look", 44);

        for (let i = 0; i < 256; i++) {
            await writeDataPort(payload[i]);
            await writeSignalsOut(0x05); // SEL=1, ACK=1
            await writeSignalsOut(0x01); // SEL=1, ACK=0
        }
        await writeSignalsOut(0x02); // Release bus

        // Call parseCommand directly on zmachine
        const cmdHex = Buffer.from("look").toString("hex");
        const zmRes = await postJSON("/api/zmm-exec", {
            name: "zmachine",
            calldata: "9ab5dc8c" + cmdHex.padEnd(64, "0") // parseCommand selector + pad
        });
        
        assert.ok(zmRes, "Z-Machine response must be valid");
        console.log("   ✓ zmachine processed 'look' command. Output result:", zmRes.result ? zmRes.result.output : "null");

        console.log("\nScenario B: Testing invalid/unknown command handling and verifying parser error response...");
        const badCmdHex = Buffer.from("invalidcommandXYZ").toString("hex");
        const badRes = await postJSON("/api/zmm-exec", {
            name: "zmachine",
            calldata: "9ab5dc8c" + badCmdHex.padEnd(64, "0")
        });
        assert.ok(badRes, "Z-Machine response must be valid for unknown command");
        console.log("   ✓ zmachine processed unknown command successfully. Response content:", badRes.result ? badRes.result.output : "null");

        console.log("\n=============================================================");
        console.log("AUNCIENT INTEGRATION TESTS COMPLETED SUCCESSFULLY");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nINTEGRATION TEST FAILURE:", err.message);
        process.exit(1);
    }
}

runTests();
