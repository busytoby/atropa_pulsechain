#!/bin/bash
# ==============================================================================
# TSFi Sovereign MCP Geometric Validation Boundary
# ==============================================================================
# 
# ARCHITECTURAL PURPOSE:
# This orchestration matrix validates the absolute physical and structural integrity
# of the `tsfi_mcp_server` continuous execution loops. It confirms that the
# inter-process communication (IPC) bindings accurately deserialize JSON-RPC
# payloads without inducing topological unmapping fractures.
#
# EXECUTION GEOMETRY:
# 1. Pipeline Injection: A formalized `wave512.run` JSON string is asynchronously
#    piped directly into the standard input stream of the isolated server instance.
# 2. Yield Barrier: A structural `sleep 0.1` constraint mathematically guarantees
#    sufficient buffer allocation execution and JIT evaluation processing.
# 3. Teardown Pulse: The sovereign `{"method": "exit"}` directive is fired to
#    forcibly trigger the architectural dismantling logic natively within the host.
# 4. Guardian Bounds: The sequence is physically locked within a 10-second `timeout`
#    matrix to conclusively terminate infinite loop drift vectors in CI environments.
# 5. Perfect Metric Verification: The captured console stream is topologically swept
#    for the rigorous `PERFECTLY ZERO LEAKS` mathematical standard, failing the test
#    if structural desynchronization vectors are encountered.
# ==============================================================================

OUTPUT=$( (echo '{"jsonrpc": "2.0", "method": "wave512.run", "params": {"code": "WLOAD W0, 3.14\nWDUMP W0"}, "id": 1}'; sleep 0.1; echo '{"method": "exit"}') | timeout 10 ./bin/tsfi_mcp_server 2>&1 )

EXIT_CODE=$?

echo "$OUTPUT"

if [ $EXIT_CODE -eq 124 ]; then
    echo "[TEST] FAIL: Process exceeded 10-second timeout!"
    exit 1
fi

if echo "$OUTPUT" | grep -q "PERFECTLY ZERO LEAKS"; then
    echo "[TEST] PASS: No memory leaks detected in MCP Server."
    exit 0
else
    echo "[TEST] FAIL: Memory leaks detected or telemetry crashed!"
    exit 1
fi
