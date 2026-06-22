const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

async function runBenchmarkAndTests() {
    console.log("=================================================");
    console.log("Auncient ZMM VM MicroUI Memory Rendering Validator");
    console.log("=================================================");

    const shmPath = "/dev/shm/atropa_raw_frame.bin";
    const width = 320;
    const height = 200;
    const frameSize = width * height * 4; // 256,000 bytes RGBA

    // 1. Benchmarking Direct Memory Frame Generation (No Puppeteer)
    console.log("[BENCHMARK] Generating 100 synthetic frames directly to memory...");
    const frameData = Buffer.alloc(frameSize);
    
    const startTime = Date.now();
    const fd = fs.openSync(shmPath, 'w');
    for (let f = 0; f < 100; f++) {
        const offset = (f * 4) % width;
        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
                const idx = (y * width + x) * 4;
                const isPattern = (x + offset) % 40 < 20 && (y + offset) % 40 < 20;
                frameData[idx] = isPattern ? 255 : 0;      // R
                frameData[idx + 1] = isPattern ? 0 : 255;  // G
                frameData[idx + 2] = 128;                  // B
                frameData[idx + 3] = 255;                  // A
            }
        }
        fs.writeSync(fd, frameData, 0, frameSize, 0);
    }
    fs.closeSync(fd);
    const elapsed = Date.now() - startTime;
    const fps = (100 / (elapsed / 1000)).toFixed(2);
    const throughput = ((frameSize * 100) / (1024 * 1024) / (elapsed / 1000)).toFixed(2);
    
    console.log(`[PASS] Write Benchmark: Generated 100 frames in ${elapsed}ms (${fps} FPS, ${throughput} MB/s)`);

    // 2. Spawn tsfi2 engine
    console.log("\n[TEST] Spawning tsfi2 CLI...");
    
    const tsfi = spawn("./tsfi2", ["--cli"], {
        cwd: path.join(__dirname, "../tsfi2-deepseek"),
        env: {
            ...process.env,
            LD_LIBRARY_PATH: ".",
            WAYLAND_DISPLAY: "wayland-1"
        }
    });

    let stdoutData = "";
    
    // Helper to wait for specific output in stdout
    const waitForStdout = (pattern, timeoutMs = 8000) => {
        return new Promise((resolve, reject) => {
            const timer = setTimeout(() => {
                reject(new Error(`Timeout waiting for stdout pattern: ${pattern}`));
            }, timeoutMs);
            
            const check = () => {
                if (stdoutData.includes(pattern)) {
                    clearTimeout(timer);
                    resolve();
                } else {
                    setTimeout(check, 100);
                }
            };
            check();
        });
    };

    tsfi.stdout.on('data', (data) => {
        stdoutData += data.toString();
        process.stdout.write(data.toString());
    });

    tsfi.stderr.on('data', (data) => {
        stdoutData += data.toString();
        process.stderr.write(data.toString());
    });

    tsfi.stdin.on('error', (err) => {
        console.error("[NODE_TEST] tsfi.stdin stream error:", err);
    });

    const writeCommand = (cmd) => {
        console.log(`[NODE_TEST] Writing to stdin: "${cmd}"`);
        tsfi.stdin.write(cmd + "\n");
        return new Promise(resolve => setTimeout(resolve, 1000));
    };

    // Load plugin
    tsfi.stdin.write("LOAD plugins/tsfi_vulkan\n");
    await waitForStdout("Successfully loaded: ./plugins/tsfi_vulkan.so");
    console.log("[TEST] Plugin compiled and loaded.");

    // Open Window
    tsfi.stdin.write("1.0 OPEN_WINDOW\n");
    await waitForStdout("Rendering Frame 0...");
    console.log("[TEST] Vulkan window mapped and frame rendering active.");

    // Inject mouse events targeting MicroUI Button 1 (X=50, Y=270)
    console.log("\n[TEST] Testing Mouse Hover/Click on MicroUI Button 1...");
    await writeCommand("1.0 TEST_MOUSE_MOVE 50 270");
    await writeCommand("1.0 TEST_MOUSE_CLICK");
    await writeCommand("1.0 TEST_MOUSE_RELEASE");

    // Inject mouse events targeting MicroUI Button 2 (X=140, Y=270)
    console.log("\n[TEST] Testing Mouse Hover/Click on MicroUI Button 2...");
    await writeCommand("1.0 TEST_MOUSE_MOVE 140 270");
    await writeCommand("1.0 TEST_MOUSE_CLICK");
    await writeCommand("1.0 TEST_MOUSE_RELEASE");

    // Gracefully exit
    console.log("\n[TEST] Shutting down tsfi2...");
    await writeCommand("0.0 EXIT");

    // Verify stdout for expected interactions
    const hasMove = stdoutData.includes("Injecting Mouse Move");
    const hasClick = stdoutData.includes("Injecting Mouse Click");
    
    if (hasMove && hasClick) {
        console.log("\n=================================================");
        console.log("[PASS] ZMM VM MicroUI Unit Tests Completed Successfully.");
        console.log("=================================================");
        process.exit(0);
    } else {
        console.error("\n=================================================");
        console.error("[FAIL] MicroUI Unit Tests Failed: Missing interaction event logs.");
        console.error("=================================================");
        process.exit(1);
    }
}

runBenchmarkAndTests().catch(err => {
    console.error("Fatal test runner error:", err);
    process.exit(1);
});
