// Unit tests for Auncient Multi-LUN Routing and Authorization Token Gate
const assert = require("assert");

console.log("=============================================================");
console.log("Auncient Multi-LUN Routing & Token Gate Verification Suite");
console.log("=============================================================");

// Simulated SCSI WinchesterMQ registers
const wmqRegisters = {
    activeLun: 0,
    isAuthorized: false,
    authCodeSeed: 0x485301a0, // Secret authorization seed
    dataPort: 0,
    routedTargets: {
        0: "AVX_THUNK",
        1: "ALSA_THUNK",
        2: "VULKAN_THUNK"
    },
    lastExecutedThunk: null
};

// 1. Authorization Token Gate (MCI calling card analogue)
function submitAuthToken(token) {
    // Verify token: expected token is the hash of the authCodeSeed with index 5
    const expectedToken = (wmqRegisters.authCodeSeed ^ 0x2600) & 0xFFFF;
    
    if (token === expectedToken) {
        wmqRegisters.isAuthorized = true;
        return true;
    }
    
    wmqRegisters.isAuthorized = false;
    return false;
}

// 2. Multi-LUN Channel Routing
function setActiveLUN(lun) {
    if (lun >= 0 && lun <= 2) {
        wmqRegisters.activeLun = lun;
        return true;
    }
    return false;
}

function writeDataPort(val) {
    if (!wmqRegisters.isAuthorized) {
        wmqRegisters.lastExecutedThunk = "BLOCKED_UNAUTHORIZED";
        return false;
    }
    
    // Route parameter write to the target mapped to the active LUN
    const target = wmqRegisters.routedTargets[wmqRegisters.activeLun];
    wmqRegisters.lastExecutedThunk = `${target}_EXECUTE_VAL_${val}`;
    return true;
}

function runAllTests() {
    try {
        console.log("1. Verifying data writes are blocked when unauthorized...");
        const writeBlocked = writeDataPort(42);
        assert.strictEqual(writeBlocked, false, "Writes must be blocked before authorization");
        assert.strictEqual(wmqRegisters.lastExecutedThunk, "BLOCKED_UNAUTHORIZED", "Status must be blocked");
        console.log("   ✓ Verification passed: Access denied for unauthorized operations.");

        console.log("2. Authorizing access using dynamic challenge token...");
        const invalidAuth = submitAuthToken(0x9999);
        assert.strictEqual(invalidAuth, false, "Invalid tokens must be rejected");
        
        const validToken = (0x485301a0 ^ 0x2600) & 0xFFFF; // Correct token
        const validAuth = submitAuthToken(validToken);
        assert.strictEqual(validAuth, true, "Valid dynamic token must be accepted");
        assert.strictEqual(wmqRegisters.isAuthorized, true, "System authorization state should be true");
        console.log("   ✓ Verification passed: Dynamic token validator engaged and authorized.");

        console.log("3. Verifying default LUN 0 routing path...");
        const writeL0 = writeDataPort(100);
        assert.strictEqual(writeL0, true);
        assert.strictEqual(wmqRegisters.lastExecutedThunk, "AVX_THUNK_EXECUTE_VAL_100", "LUN 0 must route to AVX thunk");
        console.log("   ✓ Verification passed: Default LUN 0 routed successfully.");

        console.log("4. Simulating LUN routing change to LUN 1 (ALSA Audio)...");
        setActiveLUN(1);
        const writeL1 = writeDataPort(120);
        assert.strictEqual(writeL1, true);
        assert.strictEqual(wmqRegisters.lastExecutedThunk, "ALSA_THUNK_EXECUTE_VAL_120", "LUN 1 must route to ALSA audio engine");
        console.log("   ✓ Verification passed: LUN 1 routed to ALSA audio successfully.");

        console.log("5. Simulating LUN routing change to LUN 2 (Vulkan display)...");
        setActiveLUN(2);
        const writeL2 = writeDataPort(255);
        assert.strictEqual(writeL2, true);
        assert.strictEqual(wmqRegisters.lastExecutedThunk, "VULKAN_THUNK_EXECUTE_VAL_255", "LUN 2 must route to Vulkan display buffer");
        console.log("   ✓ Verification passed: LUN 2 routed to Vulkan display successfully.");

        console.log("\n=============================================================");
        console.log("AUNCIENT MULTI-LUN ROUTING & TOKEN GATE TESTS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST FAILURE DETECTED:");
        console.error(err);
        process.exit(1);
    }
}

runAllTests();
