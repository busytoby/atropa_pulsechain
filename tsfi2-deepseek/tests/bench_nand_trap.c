#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_jpeg_encoder.h"

int main() {
    uint8_t *pixels = (uint8_t*)malloc(256 * 256 * 3);
    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;
    int iterations = 1000;

    printf("[BENCHMARK] Running %d iterations (256x256 Render + JPEG Encode)...\n", iterations);
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        generate_nand_trap_frame(pixels, i, i % 4, NULL);
        tsfi_jpeg_encode(&jpeg_buf, &jpeg_size, pixels, 256, 256, 80);
        if (jpeg_buf) {
            free(jpeg_buf); 
            jpeg_buf = NULL;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1e9;
    double fps = (double)iterations / elapsed;

    printf("  Total Time: %.3f seconds\n", elapsed);
    printf("  Performance: %.2f FPS\n", fps);

    if (fps >= 60.0) {
        printf("[SUCCESS] Met performance target (>60 FPS).\n");
    } else {
        printf("[WARNING] Performance below 60 FPS target.\n");
    }

    free(pixels);
    return 0;
}
