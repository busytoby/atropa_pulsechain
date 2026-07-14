#include "tsfi_computel_blue_box.h"
#include "lau_yul_thunk.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#define ITERATIONS 10000

int main(void) {
    // Initialize block and VM
    blue_box_init_block(1, NULL);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint8_t yul_cd[36] = {0xe3, 0x99, 0xf0, 0xe0};
    uint8_t yul_ret[32];
    size_t yul_ret_len;

    for (int i = 0; i < ITERATIONS; i++) {
        // Vary the inputs slightly to run the push-pull and Goertzel logic
        uint32_t val_in = 800 + (i % 200);
        yul_cd[35] = (uint8_t)(val_in & 0xFF);
        yul_cd[34] = (uint8_t)((val_in >> 8) & 0xFF);
        
        yul_ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", yul_cd, 36, yul_ret, &yul_ret_len);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9 + (double)(end.tv_nsec - start.tv_nsec);
    double latency_ns = elapsed_ns / ITERATIONS;

    printf("WinchesterMQ Latency: %.2f ns\n", latency_ns);
    return 0;
}
