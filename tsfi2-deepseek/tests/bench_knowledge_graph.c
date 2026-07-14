#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include "../../scripts/libantigravity_interop.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

#define ITERATIONS 1000

int main(void) {
    printf("=== Auncient Knowledge Graph & Ouroboros Performance Benchmark ===\n");

    // 1. Measure AVX-512 weight propagation
    InteropGraphEdge edges[8];
    for (int i = 0; i < 8; i++) {
        edges[i].src_agent_id = i;
        edges[i].dest_agent_id = i + 1;
        edges[i].relationship_type = 0;
        edges[i].weight = 1.0f;
        edges[i].active = 1;
    }

    double start_prop = get_time_ns();
    for (int i = 0; i < ITERATIONS * 100; i++) {
        interop_graph_propagate_weights_avx512(edges, 8, 1.001f);
    }
    double end_prop = get_time_ns();
    double prop_latency = (end_prop - start_prop) / (ITERATIONS * 100);
    printf("AVX-512 Weight Propagation Latency: %.2f ns\n", prop_latency);

    // 2. Measure Decision Tree Edge Pruning
    InteropMultiDecisionNode prune_nodes[3] = {
        { {100, 200, 300}, {1, 2, 2, 2} },
        { {0xAAAA, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} },
        { {0xBBBB, 0, 0}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} }
    };

    double start_prune = get_time_ns();
    uint32_t category = 0;
    for (int i = 0; i < ITERATIONS * 100; i++) {
        category = interop_graph_classify_edge(prune_nodes, 0, &edges[0]);
    }
    double end_prune = get_time_ns();
    (void)category;
    double prune_latency = (end_prune - start_prune) / (ITERATIONS * 100);
    printf("Decision Tree Edge Pruning Latency: %.2f ns\n", prune_latency);

    // 3. Measure NTM Path Optimization
    const char *tm_file = "temp_bench_graph_tm.dat.bin";
    InteropTMHeader tm_hdr = { 3, 2, 0, 1, 2 };
    InteropTMTransition tm_trs[2] = {
        { 0, 'a', 'b', 1, 0, 1 },
        { 1, 'x', 'y', 0, 0, 1 }
    };
    int c_res = interop_tm_compile(tm_file, &tm_hdr, tm_trs);
    assert(c_res == 0);

    uint8_t path_tape[4] = { 'a', 'x', 0, 0 };
    uint32_t path_state = 0;

    double start_ntm = get_time_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        // Reset tape for each run so path doesn't terminate instantly on failure branch
        path_tape[0] = 'a';
        path_tape[1] = 'x';
        interop_graph_optimize_paths_ntm(tm_file, path_tape, 4, &path_state);
    }
    double end_ntm = get_time_ns();
    double ntm_latency = (end_ntm - start_ntm) / ITERATIONS;
    printf("NTM Path Optimization Latency: %.2f ns\n", ntm_latency);

    remove(tm_file);
    return 0;
}
