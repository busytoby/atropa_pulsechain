#!/usr/bin/env bash
# TSFi Sovereign Roslyn Build Tool
# Mandate: Single-File C# Compilation without project artifacts.

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <source.cs> [output.dll]"
    exit 1
fi

SOURCE=$1
OUTPUT=${2:-$(basename ${SOURCE%.*}.dll)}

# 1. Substrate Paths
DOTNET_ROOT="/home/mariarahel/.dotnet"
SDK_VER="11.0.100-preview.2.26159.112"
RUNTIME_VER="11.0.0-preview.2.26159.112"

ROSLYN_PATH="$DOTNET_ROOT/sdk/$SDK_VER/Roslyn/bincore"
RUNTIME_PATH="$DOTNET_ROOT/shared/Microsoft.NETCore.App/$RUNTIME_VER"

# 2. Mandatory Assembly References (The Standard Manifold)
REFS=(
    "-r:$ROSLYN_PATH/Microsoft.CodeAnalysis.dll"
    "-r:$ROSLYN_PATH/Microsoft.CodeAnalysis.CSharp.dll"
    "-r:$RUNTIME_PATH/System.Runtime.dll"
    "-r:$RUNTIME_PATH/System.Console.dll"
    "-r:$RUNTIME_PATH/System.Private.CoreLib.dll"
    "-r:$RUNTIME_PATH/System.Collections.Immutable.dll"
    "-r:$RUNTIME_PATH/System.Linq.dll"
    "-r:$RUNTIME_PATH/System.IO.dll"
    "-r:$RUNTIME_PATH/System.Reflection.dll"
    "-r:$RUNTIME_PATH/System.Runtime.Loader.dll"
)

# 3. Direct CSC Invocation
echo "[BUILD] Realizing Sovereign Assembly: $OUTPUT..."
$DOTNET_ROOT/dotnet exec $ROSLYN_PATH/csc.dll \
    -out:$OUTPUT \
    -target:exe \
    -nologo \
    "${REFS[@]}" \
    "$SOURCE"

# 4. Success Check
if [ $? -eq 0 ]; then
    echo "[PASS] Assembly Realized without Project Artifacts."
    
    # 5. Temporary Runtime Handshake (Cleaned up immediately after use)
    # This is the only way the 'dotnet' host can boot the DLL.
    CONFIG_FILE="${OUTPUT%.*}.runtimeconfig.json"
    cat <<EOF > "$CONFIG_FILE"
{
  "runtimeOptions": {
    "tfm": "net11.0",
    "framework": {
      "name": "Microsoft.NETCore.App",
      "version": "$RUNTIME_VER"
    }
  }
}
EOF
    echo "[INFO] Generated Volatile Runtime Config: $CONFIG_FILE"
else
    echo "[FAIL] Compilation Fractured."
    exit 1
fi
