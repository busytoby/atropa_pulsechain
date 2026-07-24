#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    PATH_CPU_VECTORIZED,
    PATH_PARALLEL_COPROCESSOR
} execution_path_t;

typedef struct {
    uint32_t data_size_bytes;
    uint32_t query_complexity; // Number of predicates to evaluate
} database_query_t;

// -------------------------------------------------------------
// Pirk-Style Heterogeneous Co-processing Router
// -------------------------------------------------------------
execution_path_t route_database_query(const database_query_t *query) {
    // Holger Pirk Co-processing Decision Matrix:
    // If the data slice is small enough to fit within L1/L2 cache lines (<= 64 bytes),
    // route to CPU vectorized path to avoid PCI/coprocessor overhead.
    // If the dataset is large or highly complex, route to the parallel coprocessor.
    if (query->data_size_bytes <= 64 && query->query_complexity < 3) {
        return PATH_CPU_VECTORIZED;
    }
    return PATH_PARALLEL_COPROCESSOR;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PIRK HETEROGENEOUS CO-PROCESSING ROUTER SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Small Query Case: Fits in cache line -> CPU Vectorized
    database_query_t query_small = {
        .data_size_bytes = 16,
        .query_complexity = 1
    };

    printf("[TEST] Routing small query (16 bytes, complexity 1)...\n");
    fflush(stdout);
    execution_path_t path = route_database_query(&query_small);
    assert(path == PATH_CPU_VECTORIZED);
    printf("   ✓ Routed to CPU Vectorized Path (L1/L2 cache line match).\n");
    fflush(stdout);

    // 2. Large Query Case: Exceeds cache limit -> Parallel Coprocessor
    database_query_t query_large = {
        .data_size_bytes = 1024,
        .query_complexity = 2
    };

    printf("[TEST] Routing large query (1024 bytes, complexity 2)...\n");
    fflush(stdout);
    path = route_database_query(&query_large);
    assert(path == PATH_PARALLEL_COPROCESSOR);
    printf("   ✓ Routed to Parallel Coprocessor Path.\n");
    fflush(stdout);

    // 3. High Complexity Case: Small size but complex query -> Parallel Coprocessor
    database_query_t query_complex = {
        .data_size_bytes = 32,
        .query_complexity = 5
    };

    printf("[TEST] Routing highly complex query (32 bytes, complexity 5)...\n");
    fflush(stdout);
    path = route_database_query(&query_complex);
    assert(path == PATH_PARALLEL_COPROCESSOR);
    printf("   ✓ Routed to Parallel Coprocessor Path due to operator complexity.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PIRK CO-PROCESSING ROUTER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
