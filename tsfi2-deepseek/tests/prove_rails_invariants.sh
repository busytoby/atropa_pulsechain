#!/bin/bash
set -e

echo "============================================================="
echo "FORMAL INVARIANT PROOF RUNNER: G-15D & NCR 304 YUL HARDWARE"
echo "============================================================="

RPC_URL="http://127.0.0.1:8545"
PRIVATE_KEY="0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80"
if [ -f assets/deployed_addresses.txt ]; then
    source assets/deployed_addresses.txt
else
    G15D_ADDR="0xD748418b869a2dC4b2C20ffe64B59022f88841bb"
    NCR_ADDR="0xf516D1aC057410B67F5337717b6ADAeB127Bfdf7"
fi

# -----------------------------------------------------------------------------
# PROOF 1: G-15D Timing Sector Rotation Modulo 108 Invariant
# Invariant: S_{t+1} = (S_t + 1) mod 108
# -----------------------------------------------------------------------------
echo "[Proof 1] Verifying Timing Sector Rotation Modulo 108 Invariant..."

cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x10 0x000000000000000000000000000000000000000000000000000000000000006b > /dev/null
cast send --rpc-url $RPC_URL --private-key $PRIVATE_KEY $G15D_ADDR 0x > /dev/null
S_NEXT=$(cast storage --rpc-url $RPC_URL $G15D_ADDR 0x10)

echo "  [S_t = 107] Transitioned to: $S_NEXT"
if [ "$S_NEXT" != "0x0000000000000000000000000000000000000000000000000000000000000000" ]; then
    echo "  [FAIL] Modulo 108 wrap-around failed."
    exit 1
fi
echo "  [PASS] Proof 1 verified: timing sectors wrap correctly at 108 boundary."

# -----------------------------------------------------------------------------
# PROOF 2: NCR 304 Telemetry Parity Read-Retry Realignment Invariant
# Invariant: If retries reach the validation limit, parity bit is cleared (aligned to even)
# -----------------------------------------------------------------------------
echo "[Proof 2] Verifying Telemetry Parity Read-Retry Invariant..."

cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x28 0x0000000000000000000000000000000000000000000000000000000000000003 > /dev/null
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x22 0x0000000000000000000000000000000000000000000000000000000000000002 > /dev/null
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x21 0x0000000000000000000000000000000000000000000000000000000000000001 > /dev/null

cast send --rpc-url $RPC_URL --private-key $PRIVATE_KEY $NCR_ADDR 0x > /dev/null

RETRIES=$(cast storage --rpc-url $RPC_URL $NCR_ADDR 0x22)
PARITY=$(cast storage --rpc-url $RPC_URL $NCR_ADDR 0x21)
ERR_CODE=$(cast storage --rpc-url $RPC_URL $NCR_ADDR 0x27)

echo "  [Retries = 2, Parity = 1] Transitioned to: Retries=$RETRIES | Parity=$PARITY | ErrorCode=$ERR_CODE"

if [ "$PARITY" != "0x0000000000000000000000000000000000000000000000000000000000000000" ] || [ "$ERR_CODE" != "0x0000000000000000000000000000000000000000000000000000000000000000" ]; then
    echo "  [FAIL] Parity realignment failed."
    exit 1
fi
echo "  [PASS] Proof 2 verified: parity bit successfully realigned on retry validation limit."
echo "============================================================="
