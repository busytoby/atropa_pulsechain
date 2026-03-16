#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "tsfi_wave512.h"
#include "lau_memory.h"
#include "tsfi_wave_any.h"

// --- Legacy: "Directed" Graph (Holographic Failure Mode) ---
typedef struct Node {
    float value;
    struct Node* parent_a;
    struct Node* parent_b;
    uint8_t pad[44]; // 64-byte node
} Node;

// --- Supremacy: "Already There" Field (Bijective Waveform) ---
typedef struct {
    float value;
    int32_t hop_a; // Relative secret (atoms)
    int32_t hop_b;
    uint32_t pad;
} FieldAtom;

static double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
    size_t count = 1024 * 1024 * 4; // 4M Nodes/Atoms
    size_t bytes = count * 64;
    printf("=== ARCHITECTURAL SUPREMACY: ALREADY THERE vs DIRECTED ===\n");
    printf("Payload: %.2f MB\n", (double)bytes/1024/1024);

    // 1. Setup Directed Graph
    Node* nodes = (Node*)lau_memalign(64, count * sizeof(Node));
    for(size_t i=10; i<count; i++) {
        nodes[i].parent_a = &nodes[i-1]; nodes[i].parent_b = &nodes[i-2];
    }

    // 2. Setup Already There Field
    FieldAtom* atoms = (FieldAtom*)lau_memalign(64, count * sizeof(FieldAtom));
    for(size_t i=10; i<count; i++) {
        atoms[i].hop_a = -1; atoms[i].hop_b = -2;
    }

    // --- TEST A: Directed Traversal (Pointers) ---
    double t0 = get_time_s();
    for(size_t i=10; i<count; i++) {
        nodes[i].value = nodes[i].parent_a->value + nodes[i].parent_b->value;
    }
    double t1 = get_time_s();
    printf("  [DIRECTED] Time: %.6f s | Bandwidth: %.2f GB/s\n", t1-t0, (bytes/(t1-t0))/1e9);

    // --- TEST B: Already There Propagation (Offsets) ---
    double t2 = get_time_s();
    for(size_t i=10; i<count; i++) {
        atoms[i].value = atoms[i + atoms[i].hop_a].value + atoms[i + atoms[i].hop_b].value;
    }
    double t3 = get_time_s();
    printf("  [FIELD]    Time: %.6f s | Bandwidth: %.2f GB/s\n", t3-t2, (bytes/(t3-t2))/1e9);

    printf("  Efficiency Gain: %.2fx\n", (t1-t0)/(t3-t2));

    lau_free(nodes);
    lau_free(atoms);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
