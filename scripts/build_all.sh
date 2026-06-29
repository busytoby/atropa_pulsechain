#!/bin/bash
# Unified Build & Package Script for Dysnomia Workspace
set -e

# Resolve dotnet environment path
if [ -d "$HOME/.gemini/dotnet" ]; then
    export DOTNET_ROOT="$HOME/.gemini/dotnet"
    export PATH="$DOTNET_ROOT:$PATH"
elif [ -d "$HOME/.dotnet" ]; then
    export DOTNET_ROOT="$HOME/.dotnet"
    export PATH="$DOTNET_ROOT:$PATH"
fi

echo "=== [1/2] Rebuilding Yul Contracts & Generating compiled_yul.json ==="
node scripts/compile_all_yul.js

echo "=== [2/2] Rebuilding C# Solution & Projects ==="
if command -v dotnet &> /dev/null; then
    dotnet build Dysnomia/
else
    echo "[WARN] dotnet command not found in PATH. Skipping C# project build."
fi

echo "=== [SUCCESS] Workspace Build Complete ==="
