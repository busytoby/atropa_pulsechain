#include "../../scripts/abi_dispatch_map.c"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define ITERATIONS 50000

uint64_t bench_agent_method(void *payload, const uint64_t *args, size_t arg_count) {
    (void)payload;
    if (arg_count > 0) {
        return args[0] * 3;
    }
    return 99;
}

int main(void) {
    ABIDispatchMap map;
    abi_dispatch_init(&map);

    // Mock schema signatures
    ThunkSignature mock_schema[1] = {
        { 0x88888888, THUNK_ZMM, 1, (void*)&bench_agent_method }
    };
    
    char *raw_mem = calloc(1, 16384);
    assert(raw_mem);
    LauWiredHeader *mock_header = (LauWiredHeader*)raw_mem;
    mock_header->schema_count = 1;
    mock_header->schema = mock_schema;
    mock_header->sealed = true;
    void *mock_payload = raw_mem + 8192;

    assert(abi_dispatch_register_member(&map, mock_payload));

    struct timespec start, end;
    uint64_t args[1] = { 10 };
    uint64_t out_val = 0;

    // --- Benchmark Cache Hits ---
    // Warm up the cache first
    assert(abi_dispatch_invoke(&map, 0x88888888, mock_payload, args, 1, &out_val));

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        abi_dispatch_invoke(&map, 0x88888888, mock_payload, args, 1, &out_val);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double hit_elapsed = (double)(end.tv_sec - start.tv_sec) * 1.0e9 + (double)(end.tv_nsec - start.tv_nsec);
    double hit_latency = hit_elapsed / ITERATIONS;

    // --- Benchmark Cache Misses (with counter invalidation) ---
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < ITERATIONS; i++) {
        mock_header->counter++;
        abi_dispatch_invoke(&map, 0x88888888, mock_payload, args, 1, &out_val);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double miss_elapsed = (double)(end.tv_sec - start.tv_sec) * 1.0e9 + (double)(end.tv_nsec - start.tv_nsec);
    double miss_latency = miss_elapsed / ITERATIONS;

    printf("Agentic Dispatch Cache Hit Latency: %.2f ns\n", hit_latency);
    printf("Agentic Dispatch Cache Miss Latency: %.2f ns\n", miss_latency);

    free(raw_mem);
    return 0;
}
