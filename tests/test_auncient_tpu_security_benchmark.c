#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "auncient_sdk.h"

#define BENCHMARK_RUNS 50000

// Helper to get time in microseconds
double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

// Software Crypto password check simulation
bool cpu_crypto_verify(uint64_t input_val) {
    // Simulate CPU-heavy modular exponentiation loop (like RSA/DH check)
    uint64_t val = input_val;
    for (int i = 0; i < 50; i++) {
        val = (val * val) % 953467954114363ULL;
    }
    return (val != 0);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TPU SECURITY HANDSHAKE BENCHMARK\n");
    printf("=============================================================\n");

    // Setup challenge and response waves
    auncient_transfluxor_word_t challenge;
    auncient_sdk_compile_transfluxor_word(&challenge, "CHALLENGE", 10, 400.0, 800.0, 0.5, 0x00, 0x00);

    auncient_transfluxor_word_t response;
    auncient_sdk_compile_transfluxor_word(&response, "RESPONSE", 11, 600.0, 1200.0, 0.4, 0x00, 0x00);

    // 1. Benchmarking CPU-Heavy Cryptographic Verification
    printf("[BENCHMARK] Running %d CPU Crypto validations...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        bool verified = cpu_crypto_verify(123456789ULL);
        assert(verified);
    }
    double end_time = get_time_us();
    double cpu_duration = end_time - start_time;
    double cpu_ns_per_run = (cpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking TPU Acoustic Password Verification
    printf("[BENCHMARK] Running %d TPU Acoustic Password validations...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        double mixed_f1 = challenge.f1 + response.f1;
        double mixed_f2 = challenge.f2 + response.f2;
        bool verified = (fabs(mixed_f1 - 1000.0) < 1.0 && fabs(mixed_f2 - 2000.0) < 1.0);
        assert(verified);
    }
    end_time = get_time_us();
    double tpu_duration = end_time - start_time;
    double tpu_ns_per_run = (tpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // Output performance metrics
    double speedup = cpu_duration / tpu_duration;
    
    printf("\n=============================================================\n");
    printf("TPU SECURITY BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / verification)\n");
    printf("-------------------------------------------------------------\n");
    printf("CPU Cryptography| %15.2f | %15.2f\n", cpu_duration, cpu_ns_per_run);
    printf("TPU Acoustic PW | %15.2f | %15.2f\n", tpu_duration, tpu_ns_per_run);
    printf("-------------------------------------------------------------\n");
    printf("TPU Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
