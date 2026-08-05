#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_cache_profiler.h"
#include "hathitrust_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double hathitrust_profile_cache_latency(void) {
    // Save a dummy token to read
    const char *token_file = "/tmp/profiler_token.dat.bin";
    hathitrust_cache_save(token_file, "token123", time(NULL) + 3600);
    
    char token[256];
    
    struct timespec start, end;
    const int iterations = 1000;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < iterations; i++) {
        // Hot-path cache lookup loop (no console prints inside to avoid lock degradation)
        hathitrust_cache_load(token_file, token, sizeof(token));
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    remove(token_file);
    
    return elapsed_ns / iterations;
}
