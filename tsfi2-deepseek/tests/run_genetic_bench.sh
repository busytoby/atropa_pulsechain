#!/bin/bash
# TSFi Genetic Crossover Summary Runner

# Configuration
CFLAGS="-Iinc -O3 -march=native -pthread -flax-vector-conversions -D_POSIX_C_SOURCE=200809L -g -fsanitize=address,undefined -fno-sanitize-recover=all -DTSFI_DEBUG"
LDFLAGS="-ldl -rdynamic -lubsan -lm -fsanitize=address,undefined"
OBJS="obj/tsfi_genetic.o obj/tsfi_dysnomia.o obj/lau_memory.o obj/lau_audit.o obj/lau_registry.o obj/tsfi_math.o obj/tsfi_dys_math.o obj/tsfi_zmm_vm.o obj/tsfi_zmm_rpc.o obj/tsfi_trace.o obj/tsfi_opt_zmm.o obj/lau_thunk.o obj/tsfi_wiring.o obj/tsfi_time.o obj/tsfi_logic.o obj/tsfi_hotloader.o obj/tsfi_hilbert.o obj/tsfi_hilbert_avx512.o"

# Colors
RESET="\033[0m"
BOLD="\033[1m"
CYAN="\033[36m"
GREEN="\033[32m"

# Result storage
RESULTS=()

# Ensure objects are built
make debug > /dev/null 2>&1

run_bench() {
    local src="$1"
    local name="$2"
    local out="tests/${name}"
    
    echo -e "${CYAN}Building ${name}...${RESET}"
    gcc $CFLAGS $src $OBJS $LDFLAGS -o $out
    if [ $? -ne 0 ]; then
        echo "Failed to build ${name}"
        return 1
    fi
    
    echo -e "${CYAN}Running ${name}...${RESET}"
    local output=$( $out)
    echo "$output"
    
    # Extract throughput/score for summary
    local throughput=$(echo "$output" | grep -E "Throughput|Entropy Rate|Cascade Rate" | tail -n 1 | grep -oE "[0-9]+\.[0-9]+")
    local label=$(echo "$output" | grep -E "Throughput|Entropy Rate|Cascade Rate" | tail -n 1 | awk '{print $1}')
    
    RESULTS+=("${name}|${label}|${throughput}")
    echo ""
}

echo -e "${BOLD}=== TSFi Genetic Crossover Performance Suite ===${RESET}"
run_bench "tests/bench_mu_thetan_theta.c" "Mu_Thetan_Evo"
run_bench "tests/bench_genetic_deep.c" "Genetic_Deep"
run_bench "tests/bench_genetic_convergence_inplace.c" "Convergence_InPlace"
run_bench "tests/bench_genetic_entropy.c" "Genetic_Entropy"
run_bench "tests/bench_genetic_cascade.c" "Genetic_Cascade"

echo -e "${BOLD}=== PERFORMANCE SUMMARY ===${RESET}"
printf "%-25s | %-15s | %-20s\n" "Benchmark" "Metric" "Result"
echo "----------------------------------------------------------------------"
for res in "${RESULTS[@]}"; do
    IFS='|' read -r name label val <<< "$res"
    printf "%-25s | %-15s | %-20s\n" "$name" "$label" "$val"
done
echo "----------------------------------------------------------------------"
echo -e "${GREEN}Target: High-Frequency Neurological Evolution Sustained.${RESET}"
