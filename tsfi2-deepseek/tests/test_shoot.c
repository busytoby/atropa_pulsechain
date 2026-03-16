#include "tsfi_reaction.h"
#include "lau_memory.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <unistd.h>

static int g_current_shot = 0;

void handle_alarm(int sig) {
    (void)sig;
    tsfi_io_printf(stderr, "\n[SHOOT] TIMEOUT reached at shot %d. Exiting.\n", g_current_shot);
    exit(1);
}

int main() {
    // 10 second maximum timeout for safety
    signal(SIGALRM, handle_alarm);
    alarm(10);

    // Use the verified MotzkinPrime for the reaction
    TSFiBigInt *p_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(p_bn, 953467954114363ULL);

    int success_count = 0;
    int attempt_count = 0;

    tsfi_io_printf(stdout, "[SHOOT] Initiating 3-second YI Synthesis benchmark (Native Math)...\n");

    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    struct YI* yi_batch[8] = {NULL};
    struct Dai* result_batch[8] = {NULL};

    while (1) {
        if (attempt_count % 8000 == 0) {
            clock_gettime(CLOCK_MONOTONIC, &current);
            double elapsed = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;
            if (elapsed >= 3.0) break;
            
            tsfi_io_printf(stdout, "\r[SHOOT] Progress: %d attempts, %d successes (%.2f s)...", attempt_count, success_count, elapsed);
            tsfi_io_flush(stdout);
        }

        tsfi_reaction_shoot_batch8_fused(yi_batch, p_bn);
        attempt_count += 8;

        for (int lane = 0; lane < 8; lane++) {
            struct YI *yi = yi_batch[lane];
            if (yi == NULL || yi->Ring == NULL) continue;

            TSFiBigInt *pi_bn = yi->Ring;
            if (pi_bn->active_limbs == 0) pi_bn = yi->Xi;

            result_batch[lane] = ReactSHIO_bn_reused(result_batch[lane], yi->Psi, pi_bn);
            if (result_batch[lane]) {
                success_count++;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &current);
    double total_time = (current.tv_sec - start.tv_sec) + (current.tv_nsec - start.tv_nsec) / 1e9;

    tsfi_io_printf(stdout, "\n\n[BENCHMARK RESULTS]\n");
    tsfi_io_printf(stdout, "Total Time:    %.4f s\n", total_time);
    tsfi_io_printf(stdout, "Total Attempts: %d\n", attempt_count);
    tsfi_io_printf(stdout, "Total Success:  %d\n", success_count);
    tsfi_io_printf(stdout, "Success Rate:   %.2f%%\n", (double)success_count * 100.0 / attempt_count);
    tsfi_io_printf(stdout, "Throughput:     %.2f successful shots/sec\n", (double)success_count / total_time);
    tsfi_io_printf(stdout, "Burst Speed:    %.2f attempts/sec\n", (double)attempt_count / total_time);

    for (int i = 0; i < 8; i++) {
        if (yi_batch[i]) freeYI(yi_batch[i]);
        if (result_batch[i]) freeDAI(result_batch[i]);
    }
    tsfi_bn_free(p_bn);
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
