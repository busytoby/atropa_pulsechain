#!/bin/bash
set -e

# Robustly find the script's directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Define Project Roots
FIRMWARE_ROOT="$DIR/../../"
PULSECHAIN_ROOT="$DIR/../../../"

echo "=========================================="
echo "    ATROPA PULSECHAIN SYSTEM VERIFIER     "
echo "=========================================="
echo "Date: $(date)"
echo "------------------------------------------"

echo "[1/5] Verifying Firmware Integration (C)..."
cd "$DIR"
./build_test.sh

echo "[2/5] Verifying Native Interop Build (TSFi)..."
cd "$FIRMWARE_ROOT/tsfi_lib"
./build_interop.sh

echo "[3/5] Verifying C# Solutions (.NET 10)..."
export DOTNET_ROOT=~/.gemini/dotnet
export PATH=$DOTNET_ROOT:$PATH

cd "$PULSECHAIN_ROOT/linux"
# Build Release to ensure optimizations
dotnet build -c Release

echo "[4/5] Verifying C# Interop Logic (Tests)..."
cd "$PULSECHAIN_ROOT/tests/InteropTest"
dotnet run

echo "[5/5] Verifying Solidity Contracts..."
cd "$PULSECHAIN_ROOT/solidity"
./compile_linux.sh

echo "------------------------------------------"
echo " [SUCCESS] ALL SYSTEMS BUILDING & TESTING "
echo "=========================================="
