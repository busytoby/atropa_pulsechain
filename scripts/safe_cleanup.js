const fs = require('fs');
const path = require('path');

function safeCleanup() {
    const filePath = path.join(__dirname, "../tmp/tracked_processes.json");
    if (!fs.existsSync(filePath)) {
        console.log("[CLEANUP] No tracked processes found.");
        return;
    }

    let data = {};
    try {
        data = JSON.parse(fs.readFileSync(filePath, 'utf8'));
    } catch (e) {
        console.error("[CLEANUP] Failed to read tracked processes file:", e.message);
        return;
    }

    const pids = Object.keys(data).map(Number);
    if (pids.length === 0) {
        console.log("[CLEANUP] No active registered PIDs to clean up.");
        return;
    }

    console.log(`[CLEANUP] Terminating ${pids.length} tracked processes...`);
    for (const pid of pids) {
        const info = data[pid];
        console.log(`[CLEANUP] Killing PID ${pid} (${info.desc})`);
        try {
            process.kill(pid, 'SIGTERM');
        } catch (err) {
            if (err.code === 'ESRCH') {
                console.log(`[CLEANUP] PID ${pid} already exited.`);
            } else {
                console.error(`[CLEANUP] Failed to kill PID ${pid}:`, err.message);
                try {
                    process.kill(pid, 'SIGKILL');
                } catch (e2) {}
            }
        }
    }

    // Reset tracked file
    try {
        fs.writeFileSync(filePath, '{}', 'utf8');
        console.log("[CLEANUP] Tracked processes database cleared.");
    } catch (e) {}
}

if (require.main === module) {
    safeCleanup();
}
