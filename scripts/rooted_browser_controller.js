console.log("==================================================");
console.log("   MOCK ROOTED BROWSER CONTROLLER (PUPPETEER BANNED)   ");
console.log("==================================================");
console.log("[INFO] Puppeteer has been explicitly banned from the workspace.");
console.log("[INFO] Running dummy loop to maintain service compatibility.");

const readline = require('readline');
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false
});

rl.on('line', (line) => {
    console.log(`[MOCK RECEIVED] Input command: ${line}`);
});

// Run an infinite keeping-alive interval
setInterval(() => {}, 60000);
