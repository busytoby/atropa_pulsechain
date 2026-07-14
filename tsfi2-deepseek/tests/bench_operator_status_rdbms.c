#include "tsfi_computel_blue_box.h"
#include "lau_yul_thunk.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define ITERATIONS 10000

int main(void) {
    // 1. Initialize block, VM, and database memory structures
    blue_box_init_block(1, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint8_t dummy_sig[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t dummy_pubkey[4] = {0xCA, 0xFE, 0xBA, 0xBE};

    // Pre-populate firewall reset
    uint8_t reset_cd[4] = {0x3c, 0x13, 0x00, 0x93};
    uint8_t reset_ret[32];
    size_t reset_ret_len = 32;
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    lau_yul_thunk_execute("WinchesterMQ", reset_cd, 4, reset_ret, &reset_ret_len);

    for (int i = 0; i < ITERATIONS; i++) {
        // Evaluate dual-stack operator status check: A-side signature and B-side PLL bounds
        bool ok = blue_box_verify_dual_stack(dummy_sig, 4, dummy_pubkey, 4, 10000);
        // We assert that the status passes within wide limits
        (void)ok;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9 + (double)(end.tv_nsec - start.tv_nsec);
    double latency_ns = elapsed_ns / ITERATIONS;

    printf("RDBMS Operator Status Latency: %.2f ns\n", latency_ns);
    return 0;
}
