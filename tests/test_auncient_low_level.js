// Low-level unit tests for Auncient SCSI WinchesterMQ loopback handshake and hardware event routing
const assert = require("assert");
const net = require("net");

console.log("=============================================================");
console.log("Auncient WinchesterMQ SCSI/ZMM Event Verification Suite");
console.log("=============================================================");

// Simulated Hardware Register State Map
const yulStorage = {
    97: 0,   // MusicMaker: PIT speaker sample latch
    100: 0,  // WinchesterMQ: SCSI Status/Handshake Phase (0=IDLE, 1=REQ, 2=ACK, 3=DATA_IN, 4=DATA_OUT)
    101: 0,  // WinchesterMQ: SCSI Data latch register
    102: 0,  // WinchesterMQ: AT keyboard keycode register (30 for 'A', 32 for 'D')
    103: 1000, // WinchesterMQ: Clock divisor/damping factor
    104: 2300, // WinchesterMQ: Hyper-perspective projection focal distance
    105: 0   // WinchesterMQ: Local loopback transaction counter
};

// Raw hardware state driver implementing the Auncient SCSI handshake register loop
function processHardwareSCSIHandshake(scanCode) {
    yulStorage[100] = 1; // Assert REQ
    yulStorage[101] = scanCode; // Latch scanCode to SCSI data bus

    if (yulStorage[100] === 1 && yulStorage[101] === scanCode) {
        yulStorage[100] = 2; // Assert ACK

        if (scanCode === 30) {
            yulStorage[103] = Math.max(200, yulStorage[103] - 100);
            yulStorage[104] = Math.max(1500, yulStorage[104] - 100);
        } else if (scanCode === 32) {
            yulStorage[103] = Math.min(3000, yulStorage[103] + 100);
            yulStorage[104] = Math.min(4000, yulStorage[104] + 100);
        }

        yulStorage[105]++; // Increment SCSI transaction loopback count
        yulStorage[100] = 0; // Deassert (return to IDLE)
        return true;
    }
    return false;
}

// Low-level TCP Loopback Socket Server and Client setup to bridge raw SCSI signals
function runLoopbackSocketVerification() {
    return new Promise((resolve, reject) => {
        // Create TCP server on loopback to handle simulated raw SCSI packet transmissions
        const server = net.createServer((socket) => {
            socket.on("data", (data) => {
                // Incoming raw packet: [MagicByte0, MagicByte1, Keycode]
                if (data.length >= 3 && data[0] === 0x53 && data[1] === 0x43) { // "SC" prefix
                    const keycode = data[2];
                    const success = processHardwareSCSIHandshake(keycode);
                    
                    // Respond back with current storage state: [SuccessByte, Phase, DivisorHigh, DivisorLow]
                    const response = Buffer.alloc(4);
                    response[0] = success ? 1 : 0;
                    response[1] = yulStorage[100];
                    response[2] = (yulStorage[103] >> 8) & 0xFF;
                    response[3] = yulStorage[103] & 0xFF;
                    socket.write(response);
                }
            });
        });

        server.listen(0, "127.0.0.1", () => {
            const port = server.address().port;
            const client = net.createConnection({ port, host: "127.0.0.1" }, () => {
                // Client connected, send keycode 30 (A)
                const packet = Buffer.from([0x53, 0x43, 30]);
                client.write(packet);
            });

            client.on("data", (data) => {
                try {
                    assert.strictEqual(data[0], 1, "Handshake should succeed");
                    assert.strictEqual(data[1], 0, "Phase should be reset to IDLE (0)");
                    const finalDivisor = (data[2] << 8) | data[3];
                    assert.strictEqual(finalDivisor, 900, "Divisor for Keycode 30 should be decremented to 900");
                    assert.strictEqual(yulStorage[105], 1, "Transaction counter should be incremented");

                    // Now send keycode 32 (D)
                    const secondPacket = Buffer.from([0x53, 0x43, 32]);
                    client.write(secondPacket);
                } catch (err) {
                    cleanup(err);
                }

                client.removeAllListeners("data");
                client.on("data", (secondData) => {
                    try {
                        assert.strictEqual(secondData[0], 1, "Second handshake should succeed");
                        assert.strictEqual(secondData[1], 0, "Phase should be reset to IDLE (0)");
                        const finalDivisor = (secondData[2] << 8) | secondData[3];
                        assert.strictEqual(finalDivisor, 1000, "Divisor for Keycode 32 should be incremented back to 1000");
                        assert.strictEqual(yulStorage[105], 2, "Transaction counter should be 2");
                        cleanup(null);
                    } catch (err) {
                        cleanup(err);
                    }
                });
            });

            client.on("error", reject);

            function cleanup(err) {
                client.end();
                server.close(() => {
                    if (err) reject(err);
                    else resolve();
                });
            }
        });
    });
}

async function runAllTests() {
    try {
        console.log("1. Running unit test assertions on basic state transitions...");
        const initialCount = yulStorage[105];
        const res = processHardwareSCSIHandshake(30);
        assert.strictEqual(res, true, "Handshake function must return true");
        assert.strictEqual(yulStorage[103], 900, "Keycode 30 should decrease rotation damping divisor");
        assert.strictEqual(yulStorage[104], 2200, "Keycode 30 should decrease focal distance");
        assert.strictEqual(yulStorage[105], initialCount + 1, "Loopback transaction count should increment");
        console.log("   ✓ Basic state transition assertions passed.");

        console.log("2. Running socket loopback verification for raw SCSI bus outputs...");
        // Reset to clean states first
        yulStorage[103] = 1000;
        yulStorage[104] = 2300;
        yulStorage[105] = 0;

        await runLoopbackSocketVerification();
        console.log("   ✓ Socket loopback verification passed.");

        console.log("\n=============================================================");
        console.log("AUNCIENT LOW-LEVEL UNIT TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err);
        process.exit(1);
    }
}

runAllTests();
