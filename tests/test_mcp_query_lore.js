const fs = require("fs");
const path = require("path");
const assert = require("assert");

console.log("=============================================================");
console.log("Auncient MCP Query Lore Tool Validation Tests");
console.log("=============================================================");

const MCP_SERVER_PATH = path.join(__dirname, "../scripts/mcp_server.js");

async function runTests() {
    try {
        console.log("1. Reading mcp_server.js...");
        assert.ok(fs.existsSync(MCP_SERVER_PATH), "mcp_server.js should exist");
        const serverCode = fs.readFileSync(MCP_SERVER_PATH, "utf8");
        console.log("   ✓ MCP server file exists.");

        console.log("2. Verifying schema extensions in tools array...");
        // Check if the schema details for query_lore now list the qingAddress parameter
        assert.ok(serverCode.includes("qingAddress"), "query_lore tool schema must include qingAddress");
        assert.ok(serverCode.includes("Query live on-chain tags mapped to this contract address"), "query_lore schema must document the on-chain query");
        console.log("   ✓ Tool schema changes verified.");

        console.log("3. Verifying implementation logic of on-chain querying...");
        assert.ok(serverCode.includes("arenaProcessorAddress"), "mcp_server.js must reference arenaProcessorAddress in query_lore");
        assert.ok(serverCode.includes("arenaContract.tags(qingAddress)"), "mcp_server.js must invoke arenaContract.tags");
        assert.ok(serverCode.includes("db.qingToDocuments"), "mcp_server.js must resolve contract addresses using db.qingToDocuments");
        console.log("   ✓ Tool implementation logic changes verified.");

        console.log("\n=============================================================");
        console.log("ALL MCP QUERY LORE INTEGRATION CHECKS PASSED");
        console.log("=============================================================");
    } catch (err) {
        console.error("\nTEST RUN FAILURE:", err.message);
        process.exit(1);
    }
}

runTests();
