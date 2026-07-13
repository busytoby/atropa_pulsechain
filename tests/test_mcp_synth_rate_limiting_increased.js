// Unit tests for MCP Server Rate Limiting Verification after Threshold Increase
const assert = require("assert");

console.log("=============================================================");
console.log("AUNCIENT MCP SERVER: INCREASED RATE LIMIT THRESHOLD VERIFICATION");
console.log("=============================================================");

const ONE_HOUR_MS = 60 * 60 * 1000;

class MCPServerGasController {
    constructor() {
        this.sessions = new Map();
    }

    initializeSession(sessionId, isKnown) {
        this.sessions.set(sessionId, {
            isKnown: isKnown,
            gasLimit: isKnown ? 100000 : 5000,
            gasConsumed: 0,
            rateLimited: false,
            lastRequestTime: 0,
            requestCountInCurrentHour: 0
        });
    }

    handleRequest(sessionId, requestCost, currentTimeMs = Date.now()) {
        const session = this.sessions.get(sessionId);
        if (!session) {
            return { success: false, error: "SESSION_NOT_FOUND" };
        }

        const timePassed = currentTimeMs - session.lastRequestTime;
        if (timePassed >= ONE_HOUR_MS) {
            session.gasConsumed = 0;
            session.rateLimited = false;
            session.requestCountInCurrentHour = 0;
        }

        // Updated limit check to match server rate-limit increase (threshold = 1000 requests)
        if (!session.isKnown && session.requestCountInCurrentHour >= 1000) {
            session.rateLimited = true;
            return { success: false, error: "RATE_LIMIT_EXCEEDED_MAX_1000_PER_HOUR" };
        }

        if (session.rateLimited) {
            return { success: false, error: "RATE_LIMITED_PREVIOUSLY" };
        }

        session.gasConsumed += requestCost;
        session.requestCountInCurrentHour++;
        session.lastRequestTime = currentTimeMs;
        return { success: true, requestCount: session.requestCountInCurrentHour };
    }
}

function runMCPRateTests() {
    const server = new MCPServerGasController();
    const UNKNOWN_PEER = "session_unknown_test_01";
    const baseTime = Date.now();

    server.initializeSession(UNKNOWN_PEER, false);

    console.log("1. Simulating 5 consecutive requests from unknown peer...");
    for (let i = 0; i < 5; i++) {
        const res = server.handleRequest(UNKNOWN_PEER, 1, baseTime + i * 1000);
        assert.strictEqual(res.success, true, `Request ${i+1} should succeed`);
    }
    console.log("   ✓ Success: 5 rapid requests processed without trigger.");

    console.log("2. Simulating request count up to the limit (1000)...");
    for (let i = 5; i < 1000; i++) {
        const res = server.handleRequest(UNKNOWN_PEER, 0, baseTime + i * 1000);
        assert.strictEqual(res.success, true);
    }
    console.log("   ✓ Success: 1000 requests processed under the increased limit.");

    console.log("3. Simulating 1001st request from unknown peer...");
    const resFail = server.handleRequest(UNKNOWN_PEER, 0, baseTime + 1001 * 1000);
    assert.strictEqual(resFail.success, false, "1001st request should be rate-limited");
    assert.strictEqual(resFail.error, "RATE_LIMIT_EXCEEDED_MAX_1000_PER_HOUR");
    console.log("   ✓ Success: 1001st request blocked successfully.");

    console.log("=============================================================");
    console.log("AUNCIENT MCP INCREASED RATE LIMIT TESTS PASSED");
    console.log("=============================================================");
}

runMCPRateTests();
