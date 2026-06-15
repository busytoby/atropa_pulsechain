#!/bin/bash
set -e

# Define directories
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORKSPACE_DIR="${WORKSPACE_DIR:-$(cd "${SCRIPT_DIR}/../.." && pwd)}"
PROFILER_DIR="${WORKSPACE_DIR}/benchmarks/profiler_suite"
OUTPUT_JSON="${PROFILER_DIR}/benchmark_results.json"

echo "[INFO] Navigating to workspace directory..."
cd "${WORKSPACE_DIR}"

echo "[INFO] Compiling test_vulkan_teddy..."
make bin/test_vulkan_teddy

echo "[INFO] Ensuring output directory exists..."
mkdir -p "${PROFILER_DIR}"

echo "[INFO] Running benchmark..."
./bin/test_vulkan_teddy --benchmark

if [ -f "${OUTPUT_JSON}" ]; then
    echo "[SUCCESS] Benchmark completed successfully."
    echo "[SUCCESS] Results written to: ${OUTPUT_JSON}"
    if command -v jq >/dev/null 2>&1; then
        jq '.summary' "${OUTPUT_JSON}"
    else
        cat "${OUTPUT_JSON}" | grep -A 5 "summary"
    fi
else
    echo "[ERROR] Benchmark run did not generate the JSON results file."
    exit 1
fi
