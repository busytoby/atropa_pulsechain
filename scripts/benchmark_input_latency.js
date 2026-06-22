const net = require('net');

async function measureLatency(label, commands) {
    return new Promise((resolve, reject) => {
        const startTime = Date.now();
        const client = net.createConnection({ port: 18080, host: '127.0.0.1' }, () => {
            for (const cmd of commands) {
                client.write(cmd + '\n');
            }
            client.write('PING\n');
        });

        client.on('data', (data) => {
            if (data.toString().includes('PONG')) {
                const latency = Date.now() - startTime;
                console.log(`  [BENCHMARK] ${label}: ${latency}ms`);
                client.end();
                resolve(latency);
            }
        });

        client.on('error', (err) => {
            console.error(`  [BENCHMARK ERR] Connection failed for ${label}: ${err.message}`);
            reject(err);
        });

        client.setTimeout(2000);
        client.on('timeout', () => {
            client.destroy();
            reject(new Error("Timeout waiting for PONG"));
        });
    });
}

async function runBenchmark() {
    console.log("=================================================");
    console.log("Auncient input routing latency benchmark starting...");
    console.log("=================================================");

    try {
        // 1. Move latency (10 rapid mouse movements)
        const moveCmds = [];
        for (let i = 0; i < 10; i++) {
            moveCmds.push(`MOUSE_MOVE ${200 + i * 5} ${200 + i * 5}`);
        }
        const moveLatency = await measureLatency("10x MOUSE_MOVE queue drain", moveCmds);

        // 2. Click latency (Press & release)
        const clickCmds = [
            'MOUSE_DOWN 272 300 300',
            'MOUSE_UP 272 300 300'
        ];
        const clickLatency = await measureLatency("Mouse click (Down + Up) queue drain", clickCmds);

        // 3. Key latency (Press & release key)
        const keyCmds = [
            'KEY_DOWN 30', // Key 'a'
            'KEY_UP 30'
        ];
        const keyLatency = await measureLatency("Keystroke (Down + Up) queue drain", keyCmds);

        console.log("\n=================================================");
        console.log("AUNCIENT BENCHMARK REPORT SUMMARY");
        console.log("=================================================");
        console.log(`Average Move Latency:   ${(moveLatency / 10).toFixed(2)} ms per event`);
        console.log(`Mouse Click Latency:    ${clickLatency.toFixed(2)} ms total (2 events)`);
        console.log(`Keystroke Latency:      ${keyLatency.toFixed(2)} ms total (2 events)`);
        console.log("=================================================");
    } catch (err) {
        console.error("Benchmark execution failed:", err.message);
    }
}

runBenchmark();
