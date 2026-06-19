const { exec } = require("child_process");

console.log("[LOOP] Starting 10-cycle performance loop (runs every 4 seconds)...");

let count = 0;
const interval = setInterval(() => {
    count++;
    console.log(`[LOOP] Cycle ${count}/10 - Triggering notes...`);
    
    exec("node scripts/play_incantation_score.js", (err, stdout, stderr) => {
        if (err) {
            console.error(`Error playing score: ${err.message}`);
            return;
        }
        if (stdout) console.log(stdout.trim());
    });

    if (count >= 10) {
        clearInterval(interval);
        console.log("[LOOP] Performance loop finished.");
    }
}, 4000);
