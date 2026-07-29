const net = require('net');
const crypto = require('crypto');

console.log('=== RUNNING REAL TCP SOCKET-LEVEL STANAG OVER MCP TEST ===');

const PORT = 10045;
const DATA_SIZE = 1024 * 1024; // 1MB
const CHUNK_SIZE = 128; // 128 bytes to fit Kermit 8-bit length constraint

const sourceTelemetry = Buffer.alloc(DATA_SIZE);
for (let i = 0; i < DATA_SIZE; i++) {
    sourceTelemetry[i] = i % 256;
}

const sourceHash = crypto.createHash('sha256').update(sourceTelemetry).digest('hex');
console.log(`[TRANSMITTER] Generated 1MB telemetry data. Hash: ${sourceHash}`);

function computeKermitChecksum(buffer) {
    let sum = 0;
    for (let i = 0; i < buffer.length; i++) {
        sum += buffer[i];
    }
    return (sum & 0x3F) + 0x20;
}

function encapsulateSTANAG(seq, payload) {
    const header = Buffer.alloc(4);
    header[0] = 0x55; // STANAG Sync ID
    header[1] = 0x16; // Link-16 SAP
    header[2] = (seq >> 8) & 0xFF;
    header[3] = seq & 0xFF;
    return Buffer.concat([header, payload]);
}

function wrapKermit(seq, payload) {
    const kermitFrame = Buffer.alloc(4);
    kermitFrame[0] = 0x23; // '#'
    kermitFrame[1] = payload.length + 5;
    kermitFrame[2] = seq & 0xFF;
    kermitFrame[3] = 0x44; // 'D'
    const frameWithoutCheck = Buffer.concat([kermitFrame, payload]);
    const checkVal = computeKermitChecksum(frameWithoutCheck);
    const checksumBuf = Buffer.alloc(1);
    checksumBuf[0] = checkVal;
    return Buffer.concat([frameWithoutCheck, checksumBuf]);
}

const receivedBuffers = [];
const server = net.createServer((socket) => {
    let rawBuffer = Buffer.alloc(0);
    
    socket.on('data', (data) => {
        rawBuffer = Buffer.concat([rawBuffer, data]);
        
        while (rawBuffer.length >= 9) {
            if (rawBuffer[0] !== 0x23) {
                rawBuffer = rawBuffer.subarray(1);
                continue;
            }
            const packetLen = rawBuffer[1];
            if (rawBuffer.length < packetLen) {
                break;
            }
            
            const packet = rawBuffer.subarray(0, packetLen);
            rawBuffer = rawBuffer.subarray(packetLen);
            
            const rxChecksum = packet[packet.length - 1];
            const checkPart = packet.subarray(0, packet.length - 1);
            const expectedCheck = computeKermitChecksum(checkPart);
            
            if (rxChecksum !== expectedCheck) {
                console.log(`[RECEIVER ERROR] Parity failure on sequence ${packet[2]}. NACK!`);
                socket.write('NACK');
                return;
            }
            
            const stanagFrame = packet.subarray(4, packet.length - 1);
            const rawChunk = stanagFrame.subarray(4);
            receivedBuffers.push(rawChunk);
        }
    });
    
    socket.on('end', () => {
        const fullReceived = Buffer.concat(receivedBuffers);
        const rxHash = crypto.createHash('sha256').update(fullReceived).digest('hex');
        console.log(`[RECEIVER] Connection closed. Total received: ${fullReceived.length} bytes.`);
        console.log(`[RECEIVER] Received Data Hash: ${rxHash}`);
        
        if (rxHash === sourceHash) {
            console.log('=== [SUCCESS] BYTE-FOR-BYTE DATA PARITY VERIFIED ===');
            process.exit(0);
        } else {
            console.log('=== [FAILURE] TRANSMITTED DATA MISMATCH ===');
            process.exit(1);
        }
    });
});

server.listen(PORT, '127.0.0.1', () => {
    console.log(`[RECEIVER] Listening on Port ${PORT}...`);
    
    const client = net.createConnection({ port: PORT, host: '127.0.0.1' }, () => {
        console.log('[TRANSMITTER] Connected to Receiver coax port.');
        let offset = 0;
        let seq = 0;
        
        function sendNext() {
            if (offset >= DATA_SIZE) {
                client.end();
                return;
            }
            const chunk = sourceTelemetry.subarray(offset, offset + CHUNK_SIZE);
            const stanag = encapsulateSTANAG(seq, chunk);
            const kermit = wrapKermit(seq, stanag);
            
            client.write(kermit, () => {
                if (seq % 1024 === 0) {
                    console.log(`[TRANSMITTER] Sent sequence ${seq} (${offset} / ${DATA_SIZE} bytes).`);
                }
                offset += chunk.length;
                seq++;
                sendNext();
            });
        }
        sendNext();
    });
});
