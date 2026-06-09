// Headless Forth + 6502 Emulator Test & Benchmark Suite
const assert = require("assert");

// Mini 6502 CPU Implementation (matching cosmac_elf.html)
class CPU6502 {
    constructor() {
        this.memory = new Uint8Array(65536);
        this.pc = 0x200;
        this.a = 0;
        this.x = 0;
        this.y = 0;
        this.sp = 0xFD;
        this.flags = { n: 0, v: 0, b: 0, d: 0, i: 1, z: 1, c: 0 };
    }

    step() {
        const opcode = this.memory[this.pc];
        this.pc = (this.pc + 1) & 0xFFFF;
        switch (opcode) {
            case 0xA9: // LDA Immediate
                this.a = this.memory[this.pc];
                this.pc = (this.pc + 1) & 0xFFFF;
                this.flags.z = (this.a === 0) ? 1 : 0;
                this.flags.n = (this.a & 0x80) ? 1 : 0;
                break;
            case 0x8D: // STA Absolute
                const low = this.memory[this.pc];
                const high = this.memory[(this.pc + 1) & 0xFFFF];
                this.pc = (this.pc + 2) & 0xFFFF;
                const addr = low | (high << 8);
                this.memory[addr] = this.a;
                break;
            case 0x60: // RTS
                // Simplified return: halt PC
                this.pc = 0xFFFF;
                break;
            default:
                // Unimplemented/Halt
                this.pc = 0xFFFF;
                break;
        }
    }
}

// Mini Forth Engine parser
function runForth(line, state) {
    const tokens = line.trim().split(/\s+/);
    for (let i = 0; i < tokens.length; i++) {
        const token = tokens[i];
        if (!token) continue;
        
        // Handle hooks
        if (state.hooks[token.toLowerCase()]) {
            state.events.push(`Triggered hook: ${token}`);
            continue;
        }

        // Numeric check
        if (/^-?\d+$/.test(token) || /^0x[0-9a-fA-F]+$/.test(token)) {
            state.stack.push(parseInt(token));
        } else if (token === "flankspeed") {
            // Hex loader
            const bytesStr = tokens.slice(i + 1).join("").replace(/\s+/g, "");
            const bytes = [];
            let sum = 0;
            for (let j = 0; j < bytesStr.length; j += 2) {
                const b = parseInt(bytesStr.slice(j, j + 2), 16);
                bytes.push(b);
                sum = (sum + b) % 256;
            }
            if (state.stack.length >= 1) {
                const targetAddr = state.stack.pop();
                for (let j = 0; j < bytes.length; j++) {
                    state.cpu.memory[targetAddr + j] = bytes[j];
                }
                state.events.push(`Flankspeed loaded ${bytes.length} bytes at $${targetAddr.toString(16).toUpperCase()} (Checksum: $${sum.toString(16).toUpperCase()})`);
            }
            break; // Consume remainder of the line
        }
    }
}

// ==================== UNIT TESTS ====================
function runTests() {
    console.log("=== RUNNING EMULATOR UNIT TESTS ===");
    
    // Test 1: Stack and Parser Integration
    const state = {
        stack: [],
        hooks: {},
        cpu: new CPU6502(),
        events: []
    };
    runForth("512", state);
    assert.strictEqual(state.stack[0], 512, "Forth stack should hold 512");
    console.log("✓ Test 1: Stack Parse - PASSED");

    // Test 2: Flankspeed Memory Loading
    runForth("512 flankspeed A9 42 8D 00 03 60", state); // LDA #$42, STA $0300, RTS
    assert.strictEqual(state.cpu.memory[512], 0xA9, "Memory at 512 should be LDA opcode (0xA9)");
    assert.strictEqual(state.cpu.memory[513], 0x42, "Memory at 513 should be operand (0x42)");
    console.log("✓ Test 2: Flankspeed Memory Load - PASSED");

    // Test 3: CPU Execution
    state.cpu.pc = 512;
    let cycles = 0;
    while (state.cpu.pc !== 0xFFFF && cycles < 100) {
        state.cpu.step();
        cycles++;
    }
    assert.strictEqual(state.cpu.memory[0x0300], 0x42, "Memory at $0300 should hold written value $42");
    console.log("✓ Test 3: CPU Execution - PASSED");

    // Test 4: Hook Interception
    state.hooks["@"] = "print-at";
    runForth("@", state);
    assert.deepStrictEqual(state.events[state.events.length - 1], "Triggered hook: @", "Hook interception should trigger event");
    console.log("✓ Test 4: Hook Interception - PASSED");
}

// ==================== BENCHMARKS ====================
function runBenchmarks() {
    console.log("\n=== RUNNING CPU EMULATION BENCHMARK ===");
    const cpu = new CPU6502();
    // Pre-fill memory with infinite loop of LDA Immediate
    for (let i = 0x200; i < 0x200 + 2000; i += 2) {
        cpu.memory[i] = 0xA9;
        cpu.memory[i + 1] = 0x01;
    }
    
    const targetCycles = 20000000; // 20 Million operations
    const startTime = Date.now();
    
    for (let i = 0; i < targetCycles; i++) {
        cpu.step();
        if (cpu.pc >= 0x200 + 1990) {
            cpu.pc = 0x200; // Loop back
        }
    }
    
    const durationMs = Date.now() - startTime;
    const mips = (targetCycles / (durationMs / 1000)) / 1000000;
    
    console.log(`Executed ${targetCycles} instructions in ${durationMs}ms`);
    console.log(`Performance: ${mips.toFixed(2)} MIPS (Million Instructions Per Second)`);
}

runTests();
runBenchmarks();
