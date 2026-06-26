#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

// Functions declared in libmozilla_interop.c
extern void tsfi_compositor_cache_register(const char *key, const void *pre_data, size_t pre_size, const void *post_data, size_t post_size);
extern bool tsfi_compositor_cache_lookup(const char *pipeline_signature, void *pre_dest, size_t *pre_size_out, void *post_dest, size_t *post_size_out);

static double get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

#define BENCH_PATTERNS 100
#define BENCH_LOOKUPS 100000

int main() {
    printf("=== Auncient Compositor Caching Aho-Corasick Benchmark ===\n");

    // Generate keys
    char keys[BENCH_PATTERNS][128];
    for (int i = 0; i < BENCH_PATTERNS; i++) {
        sprintf(keys[i], "VS:layout(location=0) in vec%d pos; FS:uniform float u_time%d; FS:out vec4 color;", i % 4 + 1, i);
    }

    // 1. Measure cache registration and trie/failure links build time
    double start_build = get_time_ns();
    uint8_t dummy_data[32] = {0xAA};
    for (int i = 0; i < BENCH_PATTERNS; i++) {
        tsfi_compositor_cache_register(keys[i], dummy_data, sizeof(dummy_data), dummy_data, sizeof(dummy_data));
    }
    double end_build = get_time_ns();
    double build_time_ms = (end_build - start_build) / 1e6;
    printf("[BENCH] Build Time: %.4f ms\n", build_time_ms);

    // 2. Measure lookup throughput using Aho-Corasick
    uint8_t rx_pre[32];
    uint8_t rx_post[32];
    size_t rx_pre_sz, rx_post_sz;

    double start_lookup = get_time_ns();
    for (int i = 0; i < BENCH_LOOKUPS; i++) {
        int idx = i % BENCH_PATTERNS;
        // Search inside a simulated pipeline stream signature prefix/suffix
        char stream_sig[256];
        sprintf(stream_sig, "Pipeline: %s End", keys[idx]);
        bool found = tsfi_compositor_cache_lookup(stream_sig, rx_pre, &rx_pre_sz, rx_post, &rx_post_sz);
        assert(found);
    }
    double end_lookup = get_time_ns();
    double total_lookup_ns = end_lookup - start_lookup;
    double avg_lookup_ns = total_lookup_ns / BENCH_LOOKUPS;
    double throughput_mops = (double)BENCH_LOOKUPS / (total_lookup_ns / 1e9) / 1e6;

    printf("[BENCH] Lookup Latency: %.2f ns/lookup\n", avg_lookup_ns);
    printf("[BENCH] Throughput: %.4f M-Lookups/sec\n", throughput_mops);

    // 3. Baseline linear comparison search (strcmp baseline)
    double start_linear = get_time_ns();
    for (int i = 0; i < BENCH_LOOKUPS; i++) {
        int idx = i % BENCH_PATTERNS;
        char stream_sig[256];
        sprintf(stream_sig, "Pipeline: %s End", keys[idx]);
        
        // Linear scan baseline implementation
        int matched_idx = -1;
        for (int k = 0; k < BENCH_PATTERNS; k++) {
            if (strstr(stream_sig, keys[k]) != NULL) {
                matched_idx = k;
                break;
            }
        }
        assert(matched_idx != -1);
    }
    double end_linear = get_time_ns();
    double total_linear_ns = end_linear - start_linear;
    double avg_linear_ns = total_linear_ns / BENCH_LOOKUPS;

    printf("[BENCH] Linear Baseline: %.2f ns/lookup\n", avg_linear_ns);
    printf("[BENCH] Speedup Gain: %.2fx\n", avg_linear_ns / avg_lookup_ns);

    return 0;
}
