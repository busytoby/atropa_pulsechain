const assert = require("assert");

console.log("=============================================================");
console.log("Auncient WAGONBED: CICS to cpu6502 Oregon Trail E2E Test");
console.log("=============================================================");

// Mock the core components to verify the complete data flow pipeline cleanly without relying on active HTTP ports during unit testing.
class MockCoaxialActivityBus {
    constructor() {
        this.memory = {};
    }

    poke(address, value) {
        this.memory[address] = value;
    }

    peek(address) {
        return this.memory[address] || 0;
    }
}

class MockCICSInputReceiver {
    constructor(bus) {
        this.bus = bus;
    }

    // Emulates CICS ballistic queue injection
    injectBallisticKeystroke(keyCode) {
        // WAGONBED intercept: writes keystroke directly to tapped memory address of c6502 (e.g. address 0x0200)
        this.bus.poke(0x0200, keyCode);
    }
}

class MockCPU6502 {
    constructor(bus) {
        this.bus = bus;
        this.accumulator = 0;
    }

    executeCycle() {
        // Reads from the tapped input register (peek)
        const input = this.bus.peek(0x0200);
        
        // Emulate 6502 game logic:
        // LDA $0200 (Load input accumulator)
        this.accumulator = input;
        
        // Clear input buffer after reading (handshake reset)
        this.bus.poke(0x0200, 0);
        
        return this.accumulator;
    }
}

function runE2ETest() {
    try {
        console.log("1. Initializing Auncient Coaxial Activity Bus (ACAB) database mappings...");
        const acabBus = new MockCoaxialActivityBus();

        console.log("2. Initializing CICS input receiver with WAGONBED taps active...");
        const cicsReceiver = new MockCICSInputReceiver(acabBus);

        console.log("3. Initializing virtual cpu6502 core linked to the activity bus...");
        const cpu = new MockCPU6502(acabBus);

        console.log("4. Simulating CICS receiving keycode 32 (space bar / 'D' direction indicator)...");
        cicsReceiver.injectBallisticKeystroke(32);

        // Verify WAGONBED immediately updated the ACAB target segment
        assert.strictEqual(acabBus.peek(0x0200), 32, "WAGONBED must successfully poke keycode to target address");
        console.log("   ✓ Keystroke successfully tapped and written to bus memory address 0x0200.");

        console.log("5. Simulating cpu6502 instruction fetch and state machine processing...");
        const activeInput = cpu.executeCycle();

        // Verify the 6502 CPU register read the telemetry input
        assert.strictEqual(activeInput, 32, "CPU must read the injected keycode from memory");
        assert.strictEqual(acabBus.peek(0x0200), 0, "Input buffer must be cleared post-read");
        console.log("   ✓ cpu6502 successfully executed cycle and cleared input registers.");

        console.log("\n=============================================================");
        console.log("AUNCIENT WAGONBED CICS TO 6502 E2E TEST PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE:");
        console.error(err.message);
        process.exit(1);
    }
}

runE2ETest();
