const fs = require("fs");
const path = require("path");
const { assemble } = require("./folklore_assembler.js");

function main() {
    console.log("=== COMPILING WITCHES' SABBATH (VAESEN) FROM SOURCE ===");
    
    const asmPath = path.join(__dirname, "../frontend/witches_sabbath.asm");
    const asmSource = fs.readFileSync(asmPath, "utf8");
    const codeLines = asmSource.split("\n");
    
    console.log(`Loaded ${codeLines.length} lines of assembly source.`);
    
    try {
        const { bytes, origin } = assemble(codeLines);
        console.log(`Assembly successful! Compiled program size: ${bytes.length} bytes.`);
        console.log(`Origin Address: $${origin.toString(16).toUpperCase()}`);
        console.log(`Program Bytes: [${bytes.map(b => "0x" + b.toString(16).padStart(2, "0")).join(", ")}]`);
        
        const binPath = path.join(__dirname, "../solidity/bin/witches_sabbath.bin");
        fs.writeFileSync(binPath, Buffer.from(bytes));
        console.log(`Saved compiled binary to: ${binPath}`);
        console.log(">>> BUILD SUCCESSFUL <<<");
    } catch (err) {
        console.error("Assembly compilation failed:", err);
        process.exit(1);
    }
}

main();
