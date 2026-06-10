const { ethers } = require("ethers");
const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

const PROVIDER_URL = "http://127.0.0.1:8545";

// 6502 Opcodes
const OP = {
    LDA_IM: 0xA9, LDA_ZP: 0xA5, LDA_ABS: 0xAD,
    STA_ZP: 0x85, STA_ABS: 0x8D,
    LDX_IM: 0xA2, LDX_ZP: 0xA6, LDX_ABS: 0xAE,
    STX_ZP: 0x86, STX_ABS: 0x8E,
    LDY_IM: 0xA0, LDY_ZP: 0xA4, LDY_ABS: 0xAC,
    STY_ZP: 0x84, STY_ABS: 0x8C,
    INX: 0xE8, DEX: 0xCA,
    INY: 0xC8, DEY: 0x88,
    CLC: 0x18, SEC: 0x38,
    ADC_IM: 0x69, ADC_ZP: 0x65,
    SBC_IM: 0xE9, SBC_ZP: 0xE5,
    CMP_IM: 0xC9, CMP_ZP: 0xC5,
    BNE: 0xD0, BEQ: 0xF0,
    JMP: 0x4C, RTS: 0x60
};

function evaluateExpression(exprStr, labels) {
    let cleaned = exprStr.trim();
    if (cleaned.startsWith("#")) {
        cleaned = cleaned.slice(1);
    }
    
    // Split by operators and parenthesis while preserving them
    const tokens = cleaned.split(/([\+\-\*\/\(\)])/);
    
    const processedTokens = tokens.map(token => {
        let t = token.trim();
        if (!t) return "";
        
        // If it is a label
        if (labels[t] !== undefined) {
            return labels[t].toString();
        }
        
        // If it starts with '$' (hex)
        if (t.startsWith("$")) {
            return parseInt(t.slice(1), 16).toString();
        }
        
        // Otherwise return the token (could be number or operator)
        return t;
    }).filter(x => x);

    const valStack = [];
    const opStack = [];
    
    function getPrecedence(op) {
        if (op === "+" || op === "-") return 1;
        if (op === "*" || op === "/") return 2;
        return 0;
    }
    
    function applyOp(op, a, b) {
        if (op === "+") return a + b;
        if (op === "-") return a - b;
        if (op === "*") return a * b;
        if (op === "/") {
            if (b === 0) throw new Error("Division by zero");
            return Math.floor(a / b);
        }
        return 0;
    }
    
    for (let token of processedTokens) {
        if (token === "(") {
            opStack.push(token);
        } else if (token === ")") {
            while (opStack.length > 0 && opStack[opStack.length - 1] !== "(") {
                const op = opStack.pop();
                const b = valStack.pop();
                const a = valStack.pop();
                valStack.push(applyOp(op, a, b));
            }
            opStack.pop(); // Pop '('
        } else if (["+", "-", "*", "/"].includes(token)) {
            while (opStack.length > 0 && getPrecedence(opStack[opStack.length - 1]) >= getPrecedence(token)) {
                const op = opStack.pop();
                const b = valStack.pop();
                const a = valStack.pop();
                valStack.push(applyOp(op, a, b));
            }
            opStack.push(token);
        } else {
            if (token.startsWith("0x")) {
                valStack.push(parseInt(token, 16));
            } else {
                const parsed = parseInt(token, 10);
                if (isNaN(parsed)) {
                    throw new Error(`Invalid token in expression: ${token}`);
                }
                valStack.push(parsed);
            }
        }
    }
    
    while (opStack.length > 0) {
        const op = opStack.pop();
        const b = valStack.pop();
        const a = valStack.pop();
        valStack.push(applyOp(op, a, b));
    }
    
    return valStack[0];
}

function hasUndefinedLabels(exprStr, labels) {
    let cleaned = exprStr.replace("#", "").trim();
    const tokens = cleaned.split(/[\+\-\*\/\(\)\s]+/);
    for (let t of tokens) {
        t = t.trim();
        if (!t) continue;
        if (t.startsWith("$")) continue;
        if (!isNaN(parseInt(t, 10))) continue;
        if (t.startsWith("0x")) continue;
        if (labels[t] === undefined) {
            return true;
        }
    }
    return false;
}

function assemble(codeLines) {
    const bytes = [];
    const labels = {};
    const placeholders = []; // For absolute 16-bit target labels
    const branchPlaceholders = []; // For relative 8-bit branch labels
    const immediatePlaceholders = []; // For immediate expression values
    let origin = 0x200; 
    let currentPC = origin;

    for (let line of codeLines) {
        line = line.trim().split(";")[0].trim();
        if (!line) continue;

        // Origin directive
        if (line.toUpperCase().startsWith("ORIGIN")) {
            const tokens = line.split(/\s+/);
            origin = parseInt(tokens[1].replace("$", ""), 16);
            currentPC = origin;
            continue;
        }

        // Direct byte definitions
        if (line.toUpperCase().startsWith(".BYTE")) {
            const rawBytes = line.slice(5).split(",").map(x => parseInt(x.trim().replace("$", ""), 16));
            for (const b of rawBytes) {
                bytes.push(b);
                currentPC += 1;
            }
            continue;
        }

        if (line.endsWith(":")) {
            const labelName = line.slice(0, -1).trim();
            labels[labelName] = currentPC;
            continue;
        }

        const tokens = line.split(/\s+/);
        const mnemonic = tokens[0].toUpperCase();
        const arg = tokens.slice(1).join("");

        // Standard instructions
        if (mnemonic === "LDA") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDA_IM, 0x00);
                } else {
                    bytes.push(OP.LDA_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            } else {
                if (hasUndefinedLabels(arg, labels)) {
                    placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDA_ABS, 0x00, 0x00);
                    currentPC += 3;
                } else {
                    const val = evaluateExpression(arg, labels);
                    if (val <= 0xFF) {
                        bytes.push(OP.LDA_ZP, val);
                        currentPC += 2;
                    } else {
                        bytes.push(OP.LDA_ABS, val & 0xFF, (val >> 8) & 0xFF);
                        currentPC += 3;
                    }
                }
            }
        } else if (mnemonic === "STA") {
            if (hasUndefinedLabels(arg, labels)) {
                placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                bytes.push(OP.STA_ABS, 0x00, 0x00);
                currentPC += 3;
            } else {
                const val = evaluateExpression(arg, labels);
                if (val <= 0xFF) {
                    bytes.push(OP.STA_ZP, val);
                    currentPC += 2;
                } else {
                    bytes.push(OP.STA_ABS, val & 0xFF, (val >> 8) & 0xFF);
                    currentPC += 3;
                }
            }
        } else if (mnemonic === "LDX") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDX_IM, 0x00);
                } else {
                    bytes.push(OP.LDX_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            } else {
                if (hasUndefinedLabels(arg, labels)) {
                    placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDX_ABS, 0x00, 0x00);
                    currentPC += 3;
                } else {
                    const val = evaluateExpression(arg, labels);
                    if (val <= 0xFF) {
                        bytes.push(OP.LDX_ZP, val);
                        currentPC += 2;
                    } else {
                        bytes.push(OP.LDX_ABS, val & 0xFF, (val >> 8) & 0xFF);
                        currentPC += 3;
                    }
                }
            }
        } else if (mnemonic === "LDY") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDY_IM, 0x00);
                } else {
                    bytes.push(OP.LDY_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            } else {
                if (hasUndefinedLabels(arg, labels)) {
                    placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.LDY_ABS, 0x00, 0x00);
                    currentPC += 3;
                } else {
                    const val = evaluateExpression(arg, labels);
                    if (val <= 0xFF) {
                        bytes.push(OP.LDY_ZP, val);
                        currentPC += 2;
                    } else {
                        bytes.push(OP.LDY_ABS, val & 0xFF, (val >> 8) & 0xFF);
                        currentPC += 3;
                    }
                }
            }
        } else if (mnemonic === "STY") {
            if (hasUndefinedLabels(arg, labels)) {
                placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                bytes.push(OP.STY_ABS, 0x00, 0x00);
                currentPC += 3;
            } else {
                const val = evaluateExpression(arg, labels);
                if (val <= 0xFF) {
                    bytes.push(OP.STY_ZP, val);
                    currentPC += 2;
                } else {
                    bytes.push(OP.STY_ABS, val & 0xFF, (val >> 8) & 0xFF);
                    currentPC += 3;
                }
            }
        } else if (mnemonic === "INX") {
            bytes.push(OP.INX);
            currentPC += 1;
        } else if (mnemonic === "DEX") {
            bytes.push(OP.DEX);
            currentPC += 1;
        } else if (mnemonic === "INY") {
            bytes.push(OP.INY);
            currentPC += 1;
        } else if (mnemonic === "DEY") {
            bytes.push(OP.DEY);
            currentPC += 1;
        } else if (mnemonic === "CLC") {
            bytes.push(OP.CLC);
            currentPC += 1;
        } else if (mnemonic === "SEC") {
            bytes.push(OP.SEC);
            currentPC += 1;
        } else if (mnemonic === "ADC") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.ADC_IM, 0x00);
                } else {
                    bytes.push(OP.ADC_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            } else {
                if (hasUndefinedLabels(arg, labels)) {
                    placeholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.ADC_IM, 0x00); // Default ZP
                } else {
                    bytes.push(OP.ADC_ZP, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            }
        } else if (mnemonic === "SBC") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.SBC_IM, 0x00);
                } else {
                    bytes.push(OP.SBC_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            }
        } else if (mnemonic === "CMP") {
            if (arg.startsWith("#")) {
                if (hasUndefinedLabels(arg, labels)) {
                    immediatePlaceholders.push({ pcOffset: bytes.length + 1, target: arg });
                    bytes.push(OP.CMP_IM, 0x00);
                } else {
                    bytes.push(OP.CMP_IM, evaluateExpression(arg, labels) & 0xFF);
                }
                currentPC += 2;
            }
        } else if (mnemonic === "BNE") {
            branchPlaceholders.push({ pcOffset: bytes.length + 1, target: arg, branchPC: currentPC + 2 });
            bytes.push(OP.BNE, 0x00);
            currentPC += 2;
        } else if (mnemonic === "BEQ") {
            branchPlaceholders.push({ pcOffset: bytes.length + 1, target: arg, branchPC: currentPC + 2 });
            bytes.push(OP.BEQ, 0x00);
            currentPC += 2;
        } else if (mnemonic === "JMP") {
            placeholders.push({ pcOffset: bytes.length + 1, target: arg });
            bytes.push(OP.JMP, 0x00, 0x00);
            currentPC += 3;
        } else if (mnemonic === "RTS") {
            bytes.push(OP.RTS);
            currentPC += 1;
        }
    }

    // Resolve 16-bit Absolute placeholders (JMP, LDA, STA, etc.)
    for (const p of placeholders) {
        const val = evaluateExpression(p.target, labels);
        if (val === undefined || isNaN(val)) {
            throw new Error(`Could not resolve expression: ${p.target}`);
        }
        bytes[p.pcOffset] = val & 0xFF;
        bytes[p.pcOffset + 1] = (val >> 8) & 0xFF;
    }

    // Resolve 8-bit Immediate placeholders
    for (const p of immediatePlaceholders) {
        const val = evaluateExpression(p.target, labels);
        if (val === undefined || isNaN(val)) {
            throw new Error(`Could not resolve expression: ${p.target}`);
        }
        bytes[p.pcOffset] = val & 0xFF;
    }

    // Resolve 8-bit Relative branch placeholders (BNE, BEQ)
    for (const p of branchPlaceholders) {
        const val = evaluateExpression(p.target, labels);
        if (val === undefined || isNaN(val)) {
            throw new Error(`Could not resolve expression: ${p.target}`);
        }
        let offset = val - p.branchPC;
        if (offset < -128 || offset > 127) {
            throw new Error(`Branch expression ${p.target} out of range (offset: ${offset})`);
        }
        bytes[p.pcOffset] = offset & 0xFF;
    }

    return { bytes, origin };
}

function compileYul(filename) {
    const yulPath = path.join(__dirname, "../solidity/bin", filename);
    const solcOutput = execSync(`solc --strict-assembly "${yulPath}" --bin`, { encoding: "utf8" });
    const lines = solcOutput.split("\n");
    const binIndex = lines.findIndex(line => line.includes("Binary representation:"));
    return "0x" + lines[binIndex + 1].trim();
}

async function main() {
    const provider = new ethers.JsonRpcProvider(PROVIDER_URL);
    const signers = await provider.listAccounts();
    const signer = signers[0];

    console.log("=================================================");
    console.log("DEMO 1: Compiling & Running on CPU6502Emulator");
    console.log("=================================================");

    const srcCpu = [
        "START:",
        "LDA #$42",
        "STA $1000",
        "LDX #$0F",
        "INX",
        "JMP START"
    ];

    const { bytes: cpuBytes, origin: cpuOrigin } = assemble(srcCpu);
    console.log(`Assembled program bytes: [${cpuBytes.map(b => "0x" + b.toString(16).padStart(2, "0")).join(", ")}]`);

    // Write Demo 1 compiled bytes to demo6502_1.bin
    const cpuBinPath = path.join(__dirname, "../solidity/bin/demo6502_1.bin");
    fs.writeFileSync(cpuBinPath, Buffer.from(cpuBytes));
    console.log(`Saved compiled 6502 binary to: ${cpuBinPath}`);

    console.log("Compiling and deploying cpu6502.yul...");
    const cpuBytecode = compileYul("cpu6502.yul");
    const cpuABI = [
        "function poke(uint256 addr, uint256 val) external",
        "function peek(uint256 addr) external view returns (uint256)",
        "function runSteps(uint256 maxSteps) external returns (uint256)",
        "function getCPUState() external view returns (uint256 A, uint256 X, uint256 Y, uint256 SR, uint256 SP, uint256 PC)"
    ];

    const cpuFactory = new ethers.ContractFactory(cpuABI, cpuBytecode, signer);
    const cpuContract = await cpuFactory.deploy();
    await cpuContract.waitForDeployment();
    const cpuAddr = await cpuContract.getAddress();
    console.log(`CPU6502Emulator deployed at: ${cpuAddr}`);

    let nonce = await provider.getTransactionCount(signer.address, "pending");
    async function sendTx(contract, method, ...args) {
        const tx = await contract[method](...args, { nonce });
        nonce++;
        return tx.wait();
    }

    // Load code into CPU memory
    for (let i = 0; i < cpuBytes.length; i++) {
        await sendTx(cpuContract, "poke", cpuOrigin + i, cpuBytes[i]);
    }

    // Set PC register (0x85)
    await sendTx(cpuContract, "poke", 0x85, cpuOrigin);

    console.log("Running 10 steps of the CPU emulator...");
    await sendTx(cpuContract, "runSteps", 10);

    const state = await cpuContract.getCPUState();
    const memVal = await cpuContract.peek(0x1000);
    console.log(`\nState after execution:`);
    console.log(`Accumulator (A): 0x${state.A.toString(16).padStart(2, "0")}`);
    console.log(`X Register:      0x${state.X.toString(16).padStart(2, "0")}`);
    console.log(`PC Register:     0x${state.PC.toString(16).padStart(4, "0")}`);
    console.log(`Value at $1000:  0x${memVal.toString(16).padStart(2, "0")}`);

    console.log("\n=================================================");
    console.log("DEMO 2: Playing/Interacting on Folklore Yul CPU");
    console.log("=================================================");

    // For Folklore, let's assemble a simple game movement sequence:
    // Moves right twice, then moves left once.
    // Inputs: moves stored at 55025 (Right = 2, Left = 1)
    const srcFolk = [
        "LDA #$02",
        "STA $D6F1", // Slot 55025: move direction
        "LDA #$01",
        "STA $D6F0", // Slot 55024: trigger tick
        "LDA #$02",
        "STA $D6F1",
        "LDA #$01",
        "STA $D6F0",
        "LDA #$01",
        "STA $D6F1", // Left
        "LDA #$01",
        "STA $D6F0"
    ];
    const { bytes: folkBytes } = assemble(srcFolk);
    const folkBinPath = path.join(__dirname, "../solidity/bin/demofolklore_1.bin");
    fs.writeFileSync(folkBinPath, Buffer.from(folkBytes));
    console.log(`Saved compiled folklore binary to: ${folkBinPath}`);

    console.log("Compiling and deploying folklore.yul...");
    const folkBytecode = compileYul("folklore.yul");
    const folkABI = [
        "function poke(uint256 addr, uint256 val) external returns (uint256)",
        "function peek(uint256 addr) external view returns (uint256)"
    ];

    const folkFactory = new ethers.ContractFactory(folkABI, folkBytecode, signer);
    const folkContract = await folkFactory.deploy();
    await folkContract.waitForDeployment();
    const folkAddr = await folkContract.getAddress();
    console.log(`Folklore CPU deployed at: ${folkAddr}`);

    nonce = await provider.getTransactionCount(signer.address, "pending");

    // Let's initialize standard game settings in Folklore memory slots:
    // Slot 55027: Player X coordinate (starts at 100)
    // Slot 55028: Player Y coordinate (starts at 500)
    // Slot 55031: Player Energy (starts at 100)
    console.log("Initializing player coordinates and stats...");
    await sendTx(folkContract, "poke", 55027, 100);
    await sendTx(folkContract, "poke", 55028, 500);
    await sendTx(folkContract, "poke", 55031, 100); // 100 energy

    // Instruct moving right: Slot 55025 := 2 (Right movement)
    console.log("Commanding Player to move Right...");
    await sendTx(folkContract, "poke", 55025, 2);

    // Trigger physical environment tick: writing 1 to Slot 55024 executes physics processing on-chain
    console.log("Triggering Folklore physics cycle update...");
    await sendTx(folkContract, "poke", 55024, 1);

    // Read updated coordinates
    const px = await folkContract.peek(55027);
    const py = await folkContract.peek(55028);
    const energy = await folkContract.peek(55031);

    console.log(`\nNew coordinate: (${px}, ${py})`);
    console.log(`Player Energy:  ${energy}`);

    console.log("\n>>> SUCCESS: Both CPU and Folklore demos compiled & executed successfully! <<<");
}

main().catch(err => {
    console.error(err);
    process.exit(1);
});

module.exports = { assemble, evaluateExpression };

