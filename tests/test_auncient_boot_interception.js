// Unit tests for Auncient RAM-Based Boot Interception
const assert = require("assert");

console.log("=============================================================");
console.log("Auncient RAM-Based Boot Interception Verification Suite");
console.log("=============================================================");

// Simulated 6502 Memory Map
const memory = new Uint8Array(65536);

// Simulated WinchesterMQ SCSI Data Latch and Signal registers
const wmqStorage = {
    BSY: 0,
    REQ: 0,
    CD: 0,
    IO: 0,
    MSG: 0,
    ACK: 0,
    dataPort: 0,
    sectorBuffer: new Uint8Array(256)
};

// ROM Boot sector code at $C600 simulating loading sector data and performing jumps
const mockRomBootCode = [
    0xA2, 0x00,       // $C600: LDX #$00
    0xBD, 0x00, 0x08, // $C602: LDA $0800,X
    0x9D, 0x00, 0x02, // $C605: STA $0200,X
    0xE8,             // $C608: INX
    0xD0, 0xF5,       // $C609: BNE $C602
    0x4C, 0x01, 0x02  // $C60B: JMP $0201
];

// Write initial mock boot sector ROM to memory at $C600
for (let i = 0; i < mockRomBootCode.length; i++) {
    memory[0xC600 + i] = mockRomBootCode[i];
}

// 1. Relocate ROM Boot block to RAM ($8600)
function relocateBootROM() {
    // Simulates: 8600<C600.C6FFM
    for (let i = 0; i < 256; i++) {
        memory[0x8600 + i] = memory[0xC600 + i];
    }
    // Patch relocator exit to redirect flow (Simulates: 86F9:01 85 -> JMP $8501)
    memory[0x86F9] = 0x4C; // JMP
    memory[0x86FA] = 0x01; // Low
    memory[0x86FB] = 0x85; // High
}

// 2. Perform sector write and load via simulated SCSI data ports
function loadSectorToAddress(targetAddress, sectorData) {
    wmqStorage.BSY = 1;
    wmqStorage.CD = 0; // Data phase
    wmqStorage.IO = 0; // Write

    for (let i = 0; i < sectorData.length; i++) {
        wmqStorage.REQ = 1;
        wmqStorage.dataPort = sectorData[i];
        wmqStorage.ACK = 1; // Guest acknowledges
        
        // Latched into target memory
        memory[targetAddress + i] = wmqStorage.dataPort;
        
        wmqStorage.REQ = 0;
        wmqStorage.ACK = 0;
    }
    
    wmqStorage.BSY = 0;
}

// 3. Scan and Intercept absolute jump vector ($4C)
function interceptJumps(startAddress, scanRange) {
    for (let addr = startAddress; addr < startAddress + scanRange; addr++) {
        if (memory[addr] === 0x4C) { // JMP absolute
            const targetLow = memory[addr + 1];
            const targetHigh = memory[addr + 2];
            const originalTarget = (targetHigh << 8) | targetLow;
            
            // Redirect absolute jump to Monitor break hook ($FF5C)
            memory[addr] = 0x4C;
            memory[addr + 1] = 0x5C;
            memory[addr + 2] = 0xFF;
            
            return { interceptedAddr: addr, originalTarget };
        }
    }
    return null;
}

function runAllTests() {
    try {
        console.log("1. Verifying ROM-to-RAM boot code relocation...");
        relocateBootROM();
        assert.strictEqual(memory[0x8600], 0xA2, "LDA opcode should be copied to $8600");
        assert.strictEqual(memory[0x860B], 0x4C, "JMP opcode should be copied to $860B");
        assert.strictEqual(memory[0x86F9], 0x4C, "Patch target exit address should be updated to JMP");
        console.log("   ✓ ROM Boot code successfully relocated and patched in RAM.");

        console.log("2. Simulating SCSI disk sector transfer via WinchesterMQ data ports...");
        const sectorPayload = new Uint8Array(256);
        sectorPayload[0] = 0xEA; // NOP
        sectorPayload[1] = 0x4C; // JMP
        sectorPayload[2] = 0x00; // Low
        sectorPayload[3] = 0x03; // High ($0300)
        
        loadSectorToAddress(0x0800, sectorPayload);
        assert.strictEqual(memory[0x0800], 0xEA, "Memory at $0800 should be populated by data port write");
        assert.strictEqual(memory[0x0801], 0x4C, "Memory at $0801 should be populated by data port write");
        console.log("   ✓ SCSI data port transfer complete.");

        console.log("3. Scanning and intercepting jump vectors in guest boot space...");
        const interceptInfo = interceptJumps(0x8600, 32);
        assert.ok(interceptInfo, "Should find and intercept JMP instruction");
        assert.strictEqual(interceptInfo.originalTarget, 0x0201, "Original jump target must be $0201");
        
        // Assert modified memory points to Monitor break vector ($FF5C)
        assert.strictEqual(memory[interceptInfo.interceptedAddr], 0x4C);
        assert.strictEqual(memory[interceptInfo.interceptedAddr + 1], 0x5C);
        assert.strictEqual(memory[interceptInfo.interceptedAddr + 2], 0xFF);
        console.log(`   ✓ Intercepted JMP at $${interceptInfo.interceptedAddr.toString(16)} pointing to monitor break vector.`);

        console.log("\n=============================================================");
        console.log("AUNCIENT RAM-BASED BOOT INTERCEPTION TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err);
        process.exit(1);
    }
}

runAllTests();
