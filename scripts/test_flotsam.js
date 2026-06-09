const fs = require('fs');
const path = require('path');

const columnsPath = path.join(__dirname, 'flotsam_user_columns.json');

// Default columns
const defaultColumns = [
    {
        author: "ARTHUR DENT, DULUTH, MN",
        message: "I recently typed in Mike Schneider's Bug Repellent listing from Issue 2, but when I ran it, my C64 locked up! I suspect the SID voice registers are resonance-leaking. Please advise.",
        reply: "Ah, the classic voice leak! We checked the listing and found that you must ensure the gating bit is poked ONLY after setting the high/low frequency bounds. Also, we suggest verifying your typed program through our new TSFi2 Helmholtz proof verification command line utility: 'YULEXEC cpu6502'. Keep coding!"
    }
];

// Load existing user columns
let userColumns = [];
if (fs.existsSync(columnsPath)) {
    try {
        userColumns = JSON.parse(fs.readFileSync(columnsPath, 'utf8'));
    } catch (e) {
        userColumns = [];
    }
}

// Check if user is writing a new column
const args = process.argv.slice(2);
if (args.length >= 2) {
    const newAuthor = args[0].trim();
    const newMessage = args[1].trim();

    // Auto-generate a funny Mike Schneider reply based on message keywords
    let reply = "We've reviewed your typed listing and suggest verifying your code with our BASIC Bug Repellent check-summer. Keep coding!";
    const msgLower = newMessage.toLowerCase();
    if (msgLower.includes("sid") || msgLower.includes("sound") || msgLower.includes("music") || msgLower.includes("voice")) {
        reply = "Gating issue detected! On the C64 SID chip, you must write to the frequency registers first, and write the gating bit (bit 0 of control register $D404/$D40B/$D412) last to trigger the ADSR envelope generator.";
    } else if (msgLower.includes("yul") || msgLower.includes("helmholtz") || msgLower.includes("proof") || msgLower.includes("audit")) {
        reply = "An interesting theory on Helmholtz acoustic proofs! Try verifying your local VM state nonces using 'YULEXEC cpu6502' in the active shell.";
    } else if (msgLower.includes("bug") || msgLower.includes("crash") || msgLower.includes("lock") || msgLower.includes("freeze")) {
        reply = "Ah, the classic Bug Repellent checksum collision. Ensure you type all hexadecimal lines exactly as listed on page 42, including the trailing checksum values.";
    }

    userColumns.push({
        author: newAuthor.toUpperCase(),
        message: newMessage,
        reply: reply
    });

    try {
        fs.writeFileSync(columnsPath, JSON.stringify(userColumns, null, 2), 'utf8');
        console.log("\x1b[32m[SUCCESS] New letter submitted to Mike Schneider for review!\x1b[0m\n");
    } catch (e) {
        console.log("\x1b[31m[ERROR] Failed to save your column: " + e.message + "\x1b[0m\n");
    }
} else if (args.length === 1 && (args[0] === '-h' || args[0] === '--help')) {
    console.log("Usage: FLOTSAM \"[Your Name/Location]\" \"[Your Letter text]\"");
    console.log("Example: FLOTSAM \"Bob Lloret, NY\" \"Is there a SID sound editor coming?\"");
    process.exit(0);
}

// Print columns
console.log("\x1b[36m*** AHOY! MAGAZINE - FLOTSAM & JETSAM COLUMN ***\x1b[0m");
console.log("=================================================");

const allColumns = [...defaultColumns, ...userColumns];
allColumns.forEach((col, idx) => {
    console.log(`\x1b[33m[LETTER #${idx + 1} FROM: ${col.author}]\x1b[0m`);
    console.log(`"${col.message}"`);
    console.log("");
    console.log(`\x1b[32m[REPLY FROM TECHNICAL EDITOR MIKE SCHNEIDER]\x1b[0m`);
    console.log(`"${col.reply}"`);
    console.log("=================================================");
});
