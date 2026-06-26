// Auncient WinchesterMQ to MusicMaker and GraphicsSystem AV Loop integration test
const assert = require("assert");
const http = require("http");

console.log("=============================================================");
console.log("Auncient Closed-Loop Audio-Visual Hardware Handshake Test");
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

// Helper to call YULEXEC on a contract
async function callContract(contractName, selector, val1, val2) {
    let calldata = selector;
    if (val1 !== undefined) {
        calldata += val1.toString(16).padStart(64, "0");
    }
    if (val2 !== undefined) {
        calldata += val2.toString(16).padStart(64, "0");
    }
    const payload = {
        name: contractName,
        calldata: calldata
    };
    const res = await postJSON("/api/zmm-exec", payload);
    return res.result;
}

async function runLoopbackAVTest() {
    try {
        console.log("1. Initializing Auncient WinchesterMQ SCSI Registers...");
        // Reset bus state via writeSignalsOut(0x02) [RST=1]
        await callContract("WinchesterMQ", "485301a0", 0x02);
        
        // Assert SEL = 1 to enter select phase and trigger target response
        await callContract("WinchesterMQ", "485301a0", 0x01);

        // We will perform a series of simulated SCSI byte transmissions
        // driving the AV loop parameters.
        const simulatedBytes = [0x0A, 0x20, 0x00, 0x00, 0x01, 0x00];
        
        for (let i = 0; i < simulatedBytes.length; i++) {
            const byteVal = simulatedBytes[i];
            console.log(`\n--- Step ${i + 1}: SCSI Byte Transmit 0x${byteVal.toString(16)} ---`);
            
            // Place byte on data port
            await callContract("WinchesterMQ", "98d400c0", byteVal);
            
            // Assert ACK [SEL=1, ACK=1]
            await callContract("WinchesterMQ", "485301a0", 0x05);
            
            // Deassert ACK [SEL=1, ACK=0]
            await callContract("WinchesterMQ", "485301a0", 0x01);
            
            // Query current transient state variables updated by the SCSI target
            const bsyHex = (await callContract("WinchesterMQ", "e9d601b0", 0)).output;
            const reqHex = (await callContract("WinchesterMQ", "e9d601b0", 1)).output;
            const cdbCountHex = (await callContract("WinchesterMQ", "e9d601b0", 6)).output;
            
            const bsy = parseInt(bsyHex.trim(), 16) || 0;
            const req = parseInt(reqHex.trim(), 16) || 0;
            const cdbCount = parseInt(cdbCountHex.trim(), 16) || 0;
            
            console.log(`   Telemetry -> BSY: ${bsy}, REQ: ${req}, CDB Count: ${cdbCount}`);
            
            // Route parameters dynamically into MusicMaker (playNote)
            // playNote(uint16 note, uint8 voice) -> selector: f63eb9b8
            // Modulate note based on cdbCount, and voice based on step index
            const note = 60 + cdbCount * 2;
            const voice = (i % 3) + 1;
            console.log(`   MusicMaker: playNote(note=${note}, voice=${voice})`);
            const musicRes = await callContract("MusicMaker", "f63eb9b8", note, voice);
            assert.ok(musicRes, "MusicMaker playNote call should succeed");
            
            // Route parameters dynamically into GraphicsSystem (updateSprite)
            // updateSprite(uint8 index, uint16 x, uint8 y) -> selector: b9e48d3c
            // Modulate index based on cdbCount, x based on step index, y based on signals
            const spriteIndex = cdbCount % 8;
            const spriteX = (i + 1) * 40;
            const spriteY = 100 + (bsy * 50) + (req * 20);
            console.log(`   GraphicsSystem: updateSprite(index=${spriteIndex}, x=${spriteX}, y=${spriteY})`);
            const gfxRes = await callContract("GraphicsSystem", "b9e48d3c", spriteIndex, (spriteX << 16) | spriteY);
            assert.ok(gfxRes, "GraphicsSystem updateSprite call should succeed");
        }

        console.log("\n=============================================================");
        console.log("AUNCIENT CLOSED-LOOP AV HARDWARE INTEGRATION PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE:", err.message);
        process.exit(1);
    }
}

// Allow server startup buffering
setTimeout(() => {
    runLoopbackAVTest();
}, 1000);
