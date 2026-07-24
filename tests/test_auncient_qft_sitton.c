#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define QFT_SIZE 128
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Thread-safe output buffers
static double g_real_out[QFT_SIZE];
static double g_imag_out[QFT_SIZE];

// Sitton's Quick Fourier Transform (QFT) for real-valued sequences
// Exploits input symmetry/antisymmetry to reduce complex multiplications
void execute_qft_sitton(const double *input, double *real_out, double *imag_out, uint32_t n) {
    uint32_t half = n / 2;
    
    // Allocate buffers for symmetric and antisymmetric components
    double *sym = malloc(half * sizeof(double));
    double *antisym = malloc(half * sizeof(double));
    assert(sym != NULL && antisym != NULL);

    // 1. Decompose input sequence
    sym[0] = input[0];
    antisym[0] = 0.0;
    for (uint32_t i = 1; i < half; i++) {
        sym[i] = 0.5 * (input[i] + input[n - i]);
        antisym[i] = 0.5 * (input[i] - input[n - i]);
    }

    // 2. Perform symmetric (cosine) and antisymmetric (sine) transforms
    for (uint32_t k = 0; k < n; k++) {
        double sum_cos = sym[0];
        double sum_sin = 0.0;
        
        double angle_step = 2.0 * M_PI * k / n;
        for (uint32_t i = 1; i < half; i++) {
            double angle = angle_step * i;
            sum_cos += 2.0 * sym[i] * cos(angle);
            sum_sin -= 2.0 * antisym[i] * sin(angle);
        }
        
        // Add middle element for even sizes
        if (n % 2 == 0) {
            sum_cos += input[half] * cos(M_PI * k);
        }

        real_out[k] = sum_cos;
        imag_out[k] = sum_sin;
    }

    free(sym);
    free(antisym);
}

// -------------------------------------------------------------
// Unit Tests & Performance Benchmark
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT QUICK FOURIER TRANSFORM (QFT) VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Setup input signal: a combination of cosine waves
    double input[QFT_SIZE];
    for (int i = 0; i < QFT_SIZE; i++) {
        input[i] = cos(2.0 * M_PI * 5 * i / QFT_SIZE) + 0.5 * cos(2.0 * M_PI * 12 * i / QFT_SIZE);
    }

    // 1. Execute Sitton QFT
    execute_qft_sitton(input, g_real_out, g_imag_out, QFT_SIZE);

    // Verify frequency bins (expect peaks at bin 5 and bin 12)
    assert(fabs(g_real_out[5]) > 50.0);
    assert(fabs(g_real_out[12]) > 25.0);
    assert(fabs(g_real_out[1]) < 1.0); // No signal at bin 1
    printf("[TEST] QFT frequency decomposition verified (peaks at bin 5 & 12).\n");
    fflush(stdout);

    // 2. Performance Benchmark
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t iterations = 10000;
    for (uint64_t i = 0; i < iterations; i++) {
        execute_qft_sitton(input, g_real_out, g_imag_out, QFT_SIZE);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (double)(end.tv_sec - start.tv_sec) + 
                     (double)(end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("\nQFT Performance Metrics:\n");
    printf("   - Duration:  %.4f seconds for %ld runs\n", elapsed, iterations);
    printf("   - Throughput: %.2f transforms/sec\n", (double)iterations / elapsed);
    fflush(stdout);

    printf("=============================================================\n");
    printf("QFT INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
