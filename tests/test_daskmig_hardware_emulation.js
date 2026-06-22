// Unit tests and simulation for Auncient Daskmig Hardware Emulation
const assert = require("assert");

// VGA DAC Palette Transition Sweep Simulation (Mode 13h registers)
class VgaDacRegisterController {
    constructor() {
        this.palette = Array(256).fill(null).map(() => ({ r: 0, g: 0, b: 0 }));
        this.writeIndex = 0;
        this.colorComponentIndex = 0; // 0=R, 1=G, 2=B
    }

    // Out port emulation (Port 0x3C8 set index, Port 0x3C9 write data)
    writePort(port, val) {
        if (port === 0x3C8) {
            this.writeIndex = val & 0xFF;
            this.colorComponentIndex = 0;
        } else if (port === 0x3C9) {
            // VGA DAC color values are 6-bit (0-63)
            let colorVal = Math.min(63, val & 0x3F);
            let idx = this.writeIndex;
            if (this.colorComponentIndex === 0) {
                this.palette[idx].r = colorVal;
                this.colorComponentIndex = 1;
            } else if (this.colorComponentIndex === 1) {
                this.palette[idx].g = colorVal;
                this.colorComponentIndex = 2;
            } else if (this.colorComponentIndex === 2) {
                this.palette[idx].b = colorVal;
                // Auto-increment write index after full RGB triplet is written
                this.writeIndex = (idx + 1) & 0xFF;
                this.colorComponentIndex = 0;
            }
        }
    }

    // Emulate fading in a color target over time using Daskmig's VGA DAC palettes
    simulateFadeStep(targetColor, fadeStep, maxSteps) {
        let scale = Math.min(1, Math.max(0, fadeStep / maxSteps));
        this.writePort(0x3C8, 1); // target color index 1
        this.writePort(0x3C9, Math.round(targetColor.r * scale));
        this.writePort(0x3C9, Math.round(targetColor.g * scale));
        this.writePort(0x3C9, Math.round(targetColor.b * scale));
    }
}

// Low-Level Keyboard Scancode Parser (DOS Port 0x60 register polling)
class ScancodeParser {
    static parseScancode(code) {
        let isBreak = (code & 0x80) !== 0;
        let makeCode = code & 0x7F;

        let key = "unknown";
        if (makeCode === 0x1E) key = "A";      // makeCode 30
        else if (makeCode === 0x20) key = "D"; // makeCode 32
        else if (makeCode === 0x1C) key = "Enter";
        else if (makeCode === 0x01) key = "Escape";

        return {
            key,
            type: isBreak ? "KEY_RELEASE" : "KEY_PRESS",
            raw: code
        };
    }
}

function runTests() {
    console.log("=== RUNNING AUNCIENT DASKMIG HARDWARE EMULATION TESTS ===");

    // Test 1: VGA DAC palette write register logic
    const vga = new VgaDacRegisterController();
    vga.writePort(0x3C8, 10); // set index to 10
    vga.writePort(0x3C9, 20); // r = 20
    vga.writePort(0x3C9, 40); // g = 40
    vga.writePort(0x3C9, 60); // b = 60
    
    assert.strictEqual(vga.palette[10].r, 20);
    assert.strictEqual(vga.palette[10].g, 40);
    assert.strictEqual(vga.palette[10].b, 60);
    console.log("✓ Test 1: VGA DAC palette registers emulate correctly.");

    // Test 2: Color fade transition sweeps
    const target = { r: 60, g: 30, b: 0 };
    vga.simulateFadeStep(target, 5, 10); // step 5 of 10 -> 50% fade
    assert.strictEqual(vga.palette[1].r, 30);
    assert.strictEqual(vga.palette[1].g, 15);
    assert.strictEqual(vga.palette[1].b, 0);
    console.log("✓ Test 2: VGA DAC color transition sweeps calculate correctly.");

    // Test 3: Keyboard Port 0x60 hardware register scancode parsing
    let p1 = ScancodeParser.parseScancode(0x1E);
    assert.strictEqual(p1.key, "A");
    assert.strictEqual(p1.type, "KEY_PRESS");

    let p2 = ScancodeParser.parseScancode(0x9E);
    assert.strictEqual(p2.key, "A");
    assert.strictEqual(p2.type, "KEY_RELEASE");
    console.log("✓ Test 3: Keyboard Port 0x60 scancodes parse correctly.");

    console.log("=== ALL DASKMIG EMULATION TESTS PASSED ===");
}

runTests();
