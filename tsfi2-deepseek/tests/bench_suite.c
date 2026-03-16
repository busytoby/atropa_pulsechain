#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "tsfi_nand_trap_firmware.h"

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
    const int ITER = 1000;
    uint8_t *pixels = (uint8_t*)malloc(512 * 512 * 3);
    uint8_t *masterpiece = (uint8_t*)malloc(256 * 256 * 3);
    
    printf("=== TSFi C-Kernel Benchmark Suite ===\n");

    // Test 1: 4-Quadrant Rendering (AVX-512)
    printf("[TEST 1] 4-Quadrant 512x512 Render + Ricci Discovery...\n");
    double start = get_time();
    for (int i = 0; i < ITER; i++) {
        generate_nand_trap_frame(pixels, i, i % 4, masterpiece);
    }
    double end = get_time();
    double total = end - start;
    printf("  Total Time: %.3fs\n", total);
    printf("  Throughput: %.2f FPS\n", ITER / (total > 0 ? total : 1e-6));
    printf("  Pixel Mass: %.2f MP/s\n", (ITER * 512 * 512) / ((total > 0 ? total : 1e-6) * 1e6));

    // Test 2: Fresnel State Resolution
    printf("[TEST 2] Fresnel C/S Power Series (12 terms)...\n");
    start = get_time();
    float sum = 0;
    for (int i = 0; i < ITER * 1000; i++) {
        sum += tsfi_nand_trap_cornu_c((float)i * 0.0001f);
        sum += tsfi_nand_trap_cornu_s((float)i * 0.0001f);
    }
    end = get_time();
    printf("  Total Time: %.3fs\n", end - start);
    printf("  Latency: %.2f ns/call\n", (((end - start) / (ITER * 2000.0)) * 1e9));

    if ((ITER / (total > 0 ? total : 1e-6)) > 60.0) {
        printf("[SUCCESS] C-Kernel exceeds 60 FPS target.\n");
    } else {
        printf("[WARNING] C-Kernel below performance target.\n");
    }

    (void)sum;
    free(pixels);
    free(masterpiece);
    return 0;
}
