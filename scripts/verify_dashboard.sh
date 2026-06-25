#!/bin/bash
# Auncient Dashboard Auto-Management Verification Script
echo "=== Beginning Auncient Dashboard Verification ==="

# 1. Kill any existing instances to ensure a clean slate test
echo "[1/4] Cleaning up existing server and browser processes..."
pkill -f server.py 2>/dev/null
pkill -f rooted_browser_controller.js 2>/dev/null
sleep 2

# Verify port 8080 is indeed free
PORT_STATUS=$(lsof -i:8080)
if [ -z "$PORT_STATUS" ]; then
    echo "  -> Verified: Port 8080 is free and server is offline."
else
    echo "  -> Warning: Port 8080 is still occupied. Forcing termination..."
    fuser -k 8080/tcp 2>/dev/null
    sleep 1
fi

# 2. Launch the server and headed presenter directly
echo "[2/4] Spawning python3 server.py and presenter..."
python3 /home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/dashboard_server.py &
SERVER_PID=$!
LD_LIBRARY_PATH=/home/mariarahel/src/tsfi2/atropa_pulsechain /home/mariarahel/src/tsfi2/atropa_pulsechain/tests/rooted_frame_presenter &
PRESENTER_PID=$!

# Give the server and presenter some time to boot
echo "  -> Waiting 6 seconds for initialization..."
sleep 6

# 3. Verify server is online and responding
echo "[3/4] Testing connection to localhost dashboard..."
HTTP_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8080/nonukes)

if [ "$HTTP_STATUS" -eq 200 ]; then
    echo "  -> SUCCESS: Localhost dashboard is ONLINE and responding with HTTP 200!"
else
    echo "  -> FAILURE: Server returned HTTP status: $HTTP_STATUS"
fi

# 4. Clean up test processes
echo "[4/4] Tearing down test processes..."
kill $PRESENTER_PID 2>/dev/null
kill $SERVER_PID 2>/dev/null
pkill -f server.py 2>/dev/null
echo "=== Verification Complete ==="
