#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void run_bench(int level, int iterations) {
    printf("[ACT] Benchmarking Level %d (%d iterations)...\n", level, iterations);
    
    size_t manifold_size = (1ULL << level) * 256;
    void *manifold = lau_malloc(manifold_size);
    if (!manifold) {
        printf("  -> FAILED to allocate %zu bytes\n", manifold_size);
        return;
    }
    memset(manifold, 0, manifold_size);

    uint8_t root[32], receipt[32], sheaf[32];
    uint64_t resonance_k = 2026;

    double start = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        if (level == 0) tsfi_helmholtz_reduce_0(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 1) tsfi_helmholtz_reduce_1(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 2) tsfi_helmholtz_reduce_2(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 3) tsfi_helmholtz_reduce_3(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 4) tsfi_helmholtz_reduce_4(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 5) tsfi_helmholtz_reduce_5(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 6) tsfi_helmholtz_reduce_6(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 7) tsfi_helmholtz_reduce_7(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 8) tsfi_helmholtz_reduce_8(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 9) tsfi_helmholtz_reduce_9(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 10) tsfi_helmholtz_reduce_10(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 11) tsfi_helmholtz_reduce_11(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
        else if (level == 12) tsfi_helmholtz_reduce_12(root, receipt, sheaf, NULL, NULL, manifold, i, resonance_k, NULL);
    }
    double end = get_time_ms();

    double total_ms = end - start;
    double avg_ms = total_ms / iterations;
    double rate = 1000.0 / avg_ms;

    printf("  -> Total:   %.2f ms\n", total_ms);
    printf("  -> Average: %.4f ms\n", avg_ms);
    printf("  -> Rate:    %.2f reductions/sec\n", rate);
    printf("  -> Density: %.2f MiB/sec\n", (rate * manifold_size) / (1024.0 * 1024.0));
    
    lau_free(manifold);
}

int main() {
    printf("=== TSFi Helmholtz Scaling Analysis (Levels 0-12) ===\n");
    tsfi_helmholtz_init();

    run_bench(0, 128000);
    run_bench(1, 64000);
    run_bench(2, 32000);
    run_bench(3, 16000);
    run_bench(4, 16000);
    run_bench(5, 16000);
    run_bench(6, 16000);
    run_bench(7, 8000);
    run_bench(8, 4000);
    run_bench(9, 2000);
    run_bench(10, 1000);
    run_bench(11, 500);
    run_bench(12, 250);

    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
