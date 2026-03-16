#!/bin/bash
# Demonstrates the ability to bootstrap an internal model structure via the ZMM MCP server.

TELEMETRY_ID="XO_PROTOTYPE"
export TSFI_TELEMETRY_ID=$TELEMETRY_ID

# Start the MCP server in the background
./bin/tsfi_mcp_server > mcp_llm.log 2>&1 &
MCP_PID=$!

sleep 1 # Wait for initialization

echo "[LLM] Bootstrapping internal model structure via Genetic Crossover..."

# Define a "Baseline Architecture" (Parent A)
ARCH="WLOAD W0, 0.5
WLOAD W1, 0.1
WADD W2, W0, W1
WSTORE W2, 0"

# Define a "Strategic Input" (Parent B)
STRAT="WLOAD W0, 0.8
WLOAD W1, 0.2
WMUL W2, W0, W1
WSTORE W2, 0"

# Send the ESTABLISH_LLM request
REQUEST="{"jsonrpc": "2.0", "method": "genetic.establish_llm", "params": {"architecture": "$ARCH", "strategy": "$STRAT"}, "id": 42}"

echo "$REQUEST" | ./bin/tsfi_mcp_server | grep "result"

# Cleanup
kill $MCP_PID
echo "[LLM] Bootstrap Complete."
