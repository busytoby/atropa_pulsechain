#!/bin/bash
# Auncient Dashboard Auto-Management Verification Script
echo "=== Beginning Auncient Dashboard Verification ==="

# 1. Kill any existing instances to ensure a clean slate test
echo "[1/4] Cleaning up existing server and browser processes..."
pkill -f server.py 2>/dev/null
pkill -f rooted_browser_controller.js 2>/dev/null
sleep 2

# Verify port 8000 is indeed free
PORT_STATUS=$(lsof -i:8000)
if [ -z "$PORT_STATUS" ]; then
    echo "  -> Verified: Port 8000 is free and server is offline."
else
    echo "  -> Warning: Port 8000 is still occupied. Forcing termination..."
    fuser -k 8000/tcp 2>/dev/null
    sleep 1
fi

# 2. Launch the general browser controller without arguments
echo "[2/4] Spawning general browser controller (should auto-start the server)..."
node scripts/rooted_browser_controller.js &
CONTROLLER_PID=$!

# Give the controller and auto-spawn process some time to boot
echo "  -> Waiting 6 seconds for initialization..."
sleep 6

# 3. Verify server is online and responding
echo "[3/4] Testing connection to localhost dashboard..."
HTTP_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/atropa_dashboard.html)

if [ "$HTTP_STATUS" -eq 200 ]; then
    echo "  -> SUCCESS: Localhost dashboard is ONLINE and responding with HTTP 200!"
else
    echo "  -> FAILURE: Server returned HTTP status: $HTTP_STATUS"
fi

# 4. Clean up test processes
echo "[4/4] Tearing down test processes..."
kill $CONTROLLER_PID 2>/dev/null
pkill -f server.py 2>/dev/null
pkill -f rooted_browser_controller.js 2>/dev/null
echo "=== Verification Complete ==="
