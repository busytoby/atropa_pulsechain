// Unit tests for MCP Server Gas Rate Limiting
const assert = require("assert");

console.log("=============================================================");
console.log("AUNCIENT MCP SERVER: GAS RATE LIMITING VERIFICATION");
console.log("=============================================================");

// Simulated MCP Server state
class MCPServerGasController {
    constructor() {
        this.sessions = new Map();
    }

    // Initialize an MCP session with context-specific gas allocations
    initializeSession(sessionId, isKnown) {
        this.sessions.set(sessionId, {
            isKnown: isKnown,
            gasLimit: isKnown ? 100000 : 5000,
            gasConsumed: 0,
            rateLimited: false
        });
    }

    // Process incoming MCP request consuming gas based on payload size/complexity
    handleRequest(sessionId, requestCost) {
        const session = this.sessions.get(sessionId);
        if (!session) {
            return { success: false, error: "SESSION_NOT_FOUND" };
        }

        if (session.rateLimited) {
            return { success: false, error: "RATE_LIMITED_PREVIOUSLY" };
        }

        if (session.gasConsumed + requestCost > session.gasLimit) {
            session.rateLimited = true;
            return { success: false, error: "OUT_OF_GAS_RATE_LIMITED" };
        }

        session.gasConsumed += requestCost;
        return { success: true, gasRemaining: session.gasLimit - session.gasConsumed };
    }
}

function runMCPGasTests() {
    const server = new MCPServerGasController();

    // 1. Setup sessions
    const KNOWN_PEER = "session_known_01";
    const UNKNOWN_PEER = "session_unknown_02";

    server.initializeSession(KNOWN_PEER, true);   // Known peer (100k gas limit)
    server.initializeSession(UNKNOWN_PEER, false); // Unknown peer (5k gas limit)

    console.log("1. Simulating initial request matching (Cost: 3000 gas)...");
    
    const resKnown1 = server.handleRequest(KNOWN_PEER, 3000);
    assert.strictEqual(resKnown1.success, true, "Known peer request should succeed");
    console.log(`   ✓ Known Peer request accepted. Gas remaining: ${resKnown1.gasRemaining}`);

    const resUnknown1 = server.handleRequest(UNKNOWN_PEER, 3000);
    assert.strictEqual(resUnknown1.success, true, "Unknown peer request should succeed initially");
    console.log(`   ✓ Unknown Peer request accepted. Gas remaining: ${resUnknown1.gasRemaining}`);

    console.log("2. Simulating second request (Cost: 3000 gas)...");
    
    const resKnown2 = server.handleRequest(KNOWN_PEER, 3000);
    assert.strictEqual(resKnown2.success, true, "Known peer request should succeed (high limit)");
    console.log(`   ✓ Known Peer second request accepted. Gas remaining: ${resKnown2.gasRemaining}`);

    const resUnknown2 = server.handleRequest(UNKNOWN_PEER, 3000); // 3000 + 3000 = 6000 > 5000
    assert.strictEqual(resUnknown2.success, false, "Unknown peer request must be blocked");
    assert.strictEqual(resUnknown2.error, "OUT_OF_GAS_RATE_LIMITED", "Error must be OUT_OF_GAS_RATE_LIMITED");
    console.log("   ✓ Unknown Peer second request blocked correctly (Rate Limited).");

    console.log("3. Verifying subsequent requests from rate-limited session are blocked immediately...");
    const resUnknown3 = server.handleRequest(UNKNOWN_PEER, 100);
    assert.strictEqual(resUnknown3.success, false);
    assert.strictEqual(resUnknown3.error, "RATE_LIMITED_PREVIOUSLY");
    console.log("   ✓ Unknown Peer permanently rate-limited until session reset.");

    console.log("\n=============================================================");
    // Clean confirmation of MCP gas rate limit enforcement
    console.log("AUNCIENT MCP GAS RATE LIMITING TESTS PASSED");
    console.log("=============================================================");
}

runMCPGasTests();
