#!/bin/bash
set -e

echo "============================================================="
echo "TSFi AUNCIENT: RAILS EVM YUL HARDWARE DAT INTEGRATION TEST"
echo "============================================================="

RPC_URL="http://127.0.0.1:8545"
PRIVATE_KEY="0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80"

echo "[Test] Verifying connectivity to Anvil RPC node..."
cast block-number --rpc-url $RPC_URL
echo "  [PASS] Anvil RPC is online."

# 1. Run local serialization trace to generate DAT assets, compile & deploy new Yul contracts dynamically from C
echo "[Test] Executing C-level Double-Array Trie trace serialization..."
./tests/test_serialization_dat > /dev/null
echo "  [PASS] Trace generated successfully."

# Load dynamically deployed addresses from the generated manifest file
if [ -f assets/deployed_addresses.txt ]; then
    source assets/deployed_addresses.txt
    echo "  [INFO] Interfacing with dynamically deployed contracts: G15D=$G15D_ADDR, NCR=$NCR_ADDR"
else
    echo "  [ERROR] Deployed address manifest assets/deployed_addresses.txt not found!"
    exit 1
fi

# 2. Initialize G-15D (Black Rail) register states on EVM using anvil_setStorageAt
echo "[Test] Initializing G-15D (Black Rail) register states on EVM..."
# Reset PC (slot 0x01) to 0
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x01 0x0000000000000000000000000000000000000000000000000000000000000000 > /dev/null
# Reset drumTiming sector (slot 0x10) to 0
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x10 0x0000000000000000000000000000000000000000000000000000000000000000 > /dev/null
# Reset Double Precision Mode (slot 0x13) to 0
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x13 0x0000000000000000000000000000000000000000000000000000000000000000 > /dev/null
# Set Base Reg (slot 0x50) to 10
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x50 0x000000000000000000000000000000000000000000000000000000000000000a > /dev/null
# Set DDA Scaling Coefficient (slot 0x15) to 5
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $G15D_ADDR 0x15 0x0000000000000000000000000000000000000000000000000000000000000005 > /dev/null

# 3. Invoke G-15D execution cycle
echo "[Test] Triggering G-15D cycle transitions..."
cast send --rpc-url $RPC_URL --private-key $PRIVATE_KEY $G15D_ADDR 0x --gas-limit 100000 > /dev/null

# Read updated values from Anvil storage
SECTOR_VAL=$(cast storage --rpc-url $RPC_URL $G15D_ADDR 0x10)
BASE_REG_VAL=$(cast storage --rpc-url $RPC_URL $G15D_ADDR 0x50)

echo "  [INFO] G-15D Drum Sector: $SECTOR_VAL"
echo "  [INFO] G-15D DDA Scaled Base Register: $BASE_REG_VAL"

# 4. Initialize NCR 304 (Red Rail) register states on EVM using anvil_setStorageAt
echo "[Test] Initializing NCR 304 (Red Rail) register states on EVM..."
# Set retry limit (slot 0x28) to 3
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x28 0x0000000000000000000000000000000000000000000000000000000000000003 > /dev/null
# Set parity check (slot 0x21) to 0 (even, no parity warning)
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x21 0x0000000000000000000000000000000000000000000000000000000000000000 > /dev/null
# Set Multi-Channel Tape Merge-Sorter active (slot 0x2a) to 1
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x2a 0x0000000000000000000000000000000000000000000000000000000000000001 > /dev/null
# Set Card-to-Tape Transcoder Status (slot 0x2b) to 0
cast rpc --rpc-url $RPC_URL anvil_setStorageAt $NCR_ADDR 0x2b 0x0000000000000000000000000000000000000000000000000000000000000000 > /dev/null

echo "[Test] Triggering NCR 304 cycle transitions..."
cast send --rpc-url $RPC_URL --private-key $PRIVATE_KEY $NCR_ADDR 0x --gas-limit 100000 > /dev/null

TRANSCODER_VAL=$(cast storage --rpc-url $RPC_URL $NCR_ADDR 0x2b)
echo "  [INFO] NCR 304 Transcoding Status: $TRANSCODER_VAL"

# Validate outcomes
if [ "$BASE_REG_VAL" != "0x000000000000000000000000000000000000000000000000000000000000000f" ]; then
    echo "  [FAIL] DDA Scaling integration check failed."
    exit 1
fi
if [ "$TRANSCODER_VAL" != "0x0000000000000000000000000000000000000000000000000000000000000001" ]; then
    echo "  [FAIL] Tape merge transcoding status progression failed."
    exit 1
fi

echo "  [PASS] All EVM Yul Rail hardware transitions validated successfully!"
echo "============================================================="
