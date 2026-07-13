// Unit tests for MCP Server Gas Rate Limiting & Hourly Reset Logic
const assert = require("assert");

console.log("=============================================================");
console.log("AUNCIENT MCP SERVER: HOURLY GAS RATE LIMITING VERIFICATION");
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

    // Process request with hourly rate limit check
    handleRequest(sessionId, requestCost, currentTimeMs = Date.now()) {
        const session = this.sessions.get(sessionId);
        if (!session) {
            return { success: false, error: "SESSION_NOT_FOUND" };
        }

        // Check if 1 hour has passed since last request to reset gas
        const timePassed = currentTimeMs - session.lastRequestTime;
        if (timePassed >= ONE_HOUR_MS) {
            session.gasConsumed = 0;
            session.rateLimited = false;
            session.requestCountInCurrentHour = 0;
        }

        // Enforce 1-request-per-hour limit for unknown peers
        if (!session.isKnown && session.requestCountInCurrentHour >= 1) {
            session.rateLimited = true;
            return { success: false, error: "RATE_LIMIT_EXCEEDED_MAX_1_PER_HOUR" };
        }

        if (session.rateLimited) {
            return { success: false, error: "RATE_LIMITED_PREVIOUSLY" };
        }

        if (session.gasConsumed + requestCost > session.gasLimit) {
            session.rateLimited = true;
            return { success: false, error: "OUT_OF_GAS_RATE_LIMITED" };
        }

        session.gasConsumed += requestCost;
        session.requestCountInCurrentHour++;
        session.lastRequestTime = currentTimeMs;
        return { success: true, gasRemaining: session.gasLimit - session.gasConsumed };
    }
}

function runMCPGasTests() {
    const server = new MCPServerGasController();

    const KNOWN_PEER = "session_known_01";
    const UNKNOWN_PEER = "session_unknown_02";

    const baseTime = Date.now();

    server.initializeSession(KNOWN_PEER, true);   // Known peer (100k gas limit)
    server.initializeSession(UNKNOWN_PEER, false); // Unknown peer (5k gas limit, 1 req/hour)

    console.log("1. Simulating initial request from unknown peer...");
    const res1 = server.handleRequest(UNKNOWN_PEER, 3000, baseTime);
    assert.strictEqual(res1.success, true, "First request should succeed");
    console.log("   ✓ Unknown Peer first request succeeded.");

    console.log("2. Simulating second request from unknown peer within same hour...");
    const res2 = server.handleRequest(UNKNOWN_PEER, 100, baseTime + 1000); // 1 second later
    assert.strictEqual(res2.success, false, "Second request within 1 hour must fail");
    assert.strictEqual(res2.error, "RATE_LIMIT_EXCEEDED_MAX_1_PER_HOUR");
    console.log("   ✓ Unknown Peer second request blocked correctly (Rate Limit Exceeded).");

    console.log("3. Simulating request from unknown peer after 1 hour has elapsed...");
    const oneHourLater = baseTime + ONE_HOUR_MS + 1000;
    const res3 = server.handleRequest(UNKNOWN_PEER, 3000, oneHourLater);
    assert.strictEqual(res3.success, true, "Request after 1 hour must succeed (reset elapsed)");
    console.log("   ✓ Unknown Peer gas limit and request count successfully reset after 1 hour.");

    console.log("\n=============================================================");
    console.log("AUNCIENT MCP HOURLY GAS RATE LIMITING TESTS PASSED");
    console.log("=============================================================");
}

runMCPGasTests();
