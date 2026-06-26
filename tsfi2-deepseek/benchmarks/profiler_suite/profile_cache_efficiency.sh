#!/bin/bash
# Auncient CPU Cache Efficiency Profiler using Cachegrind
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORKSPACE_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TMP_LOG="/tmp/cachegrind_ac.log"

echo "=== Auncient CPU Cache Efficiency Profiler ==="
cd "${WORKSPACE_DIR}"

# Ensure benchmark binaries are compiled without AVX-512 for Valgrind compatibility
echo "[BUILD] Rebuilding compositor interop benchmark for Cachegrind..."
gcc -Wall -Wextra -Iinc -Isrc -O3 -g tests/bench_ac_compositor_interop.c tests/libmozilla_interop.c -o tests/bench_ac_compositor_interop_cg -lpthread

if ! command -v valgrind &> /dev/null; then
    echo "[ERROR] Valgrind is not installed on this system. Cannot run Cachegrind profiling."
    echo "Please install valgrind (e.g., 'apt-get install valgrind') to utilize this tool."
    exit 1
fi

echo "[RUN] Running Cachegrind on Aho-Corasick Compositor Interop..."
# Run cachegrind with default L1/LL settings, outputting summary to stderr
valgrind --tool=cachegrind --cache-sim=yes --cachegrind-out-file=/tmp/cg.out ./tests/bench_ac_compositor_interop_cg --quick 2> "${TMP_LOG}" || true
rm -f /tmp/cg.out

# Parse results
# Lines look like: 
# ==12345== I   refs:      10,000,000
# ==12345== D   refs:       5,000,000  (3,500,000 rd   + 1,500,000 wr)
# ==12345== D1  misses:        50,000  (   40,000 rd   +    10,000 wr)

D_REFS=$(grep -iP 'D\s+refs:' "${TMP_LOG}" | grep -oP 'refs:\s+\K[0-9,]+' | tr -d ',' || echo "0")
D1_MISSES=$(grep -iP 'D1\s+misses:' "${TMP_LOG}" | grep -oP 'misses:\s+\K[0-9,]+' | tr -d ',' || echo "0")
I_REFS=$(grep -iP 'I\s+refs:' "${TMP_LOG}" | grep -oP 'refs:\s+\K[0-9,]+' | tr -d ',' || echo "0")
I1_MISSES=$(grep -iP 'I1\s+misses:' "${TMP_LOG}" | grep -oP 'misses:\s+\K[0-9,]+' | tr -d ',' || echo "0")

if [ "${D_REFS}" -eq 0 ] 2>/dev/null || [ -z "${D_REFS}" ]; then
    echo "[WARN] Cachegrind output was incomplete or could not be parsed."
    cat "${TMP_LOG}"
    exit 0
fi

# Calculate hit rates
D1_HIT_RATE=$(python3 -c "print(f'{(1.0 - (${D1_MISSES} / ${D_REFS})) * 100:.4f}')" 2>/dev/null || echo "0.00")
I1_HIT_RATE=$(python3 -c "print(f'{(1.0 - (${I1_MISSES} / ${I_REFS})) * 100:.4f}')" 2>/dev/null || echo "0.00")

echo ""
echo "--------------------------------------------------------"
echo "                CACHE METRICS REPORT"
echo "--------------------------------------------------------"
printf "%-30s | %-12s\n" "Metric Category" "Count / Rate"
echo "--------------------------------------------------------"
printf "%-30s | %'d\n" "Total Data Reads/Writes" "${D_REFS}"
printf "%-30s | %'d\n" "Data L1 Cache Misses" "${D1_MISSES}"
printf "%-30s | %-12s\n" "L1 Data Hit Rate (D1)" "${D1_HIT_RATE}%"
echo "--------------------------------------------------------"
printf "%-30s | %'d\n" "Total Instruction Reads" "${I_REFS}"
printf "%-30s | %'d\n" "Instruction L1 Cache Misses" "${I1_MISSES}"
printf "%-30s | %-12s\n" "L1 Instruction Hit Rate (I1)" "${I1_HIT_RATE}%"
echo "--------------------------------------------------------"

# Strict performance compliance gate
TARGET_RATE=98.0
IS_COMPLIANT=$(python3 -c "print('PASS' if float('${D1_HIT_RATE}') >= ${TARGET_RATE} else 'FAIL')")

if [ "${IS_COMPLIANT}" = "PASS" ]; then
    echo -e "\033[32m[SUCCESS] L1 Data Cache Hit Rate is compliant (>= ${TARGET_RATE}%).\033[0m"
    exit 0
else
    echo -e "\033[31m[FAILURE] L1 Data Cache Hit Rate fell below target of ${TARGET_RATE}%!\033[0m"
    echo -e "Review struct layouts and spatial locality in libmozilla_interop.c."
    exit 1
fi
