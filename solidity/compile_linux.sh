#!/bin/bash
# Compile all solidity contracts in dysnomia/ directory
set -e

# Ensure output directory exists
mkdir -p ../Wallet/bin/Contracts

echo "[SOLC] Starting compilation..."

# Find all .sol files in dysnomia/ excluding interfaces and old
find dysnomia -type f -name "*.sol" | grep -v "interfaces" | grep -v "/old/" | while read -r i; do
    echo "[SOLC] Compiling $i"
    solc --combined-json=bin,abi --optimize --optimize-runs=200 --evm-version=shanghai "$i" > "../Wallet/bin/Contracts/$(basename "$i").json"
done

echo "[SOLC] Compilation complete."
