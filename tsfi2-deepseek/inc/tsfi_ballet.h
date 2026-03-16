#ifndef TSFI_BALLET_H
#define TSFI_BALLET_H

#include <stdio.h>
#include <stdlib.h>
#include "lau_memory.h"

/**
 * TSFi Ballet Audit: Detects memory leaks as early as possible.
 * Compares current active allocations against a provided checkpoint.
 */
static inline void ballet_audit(size_t checkpoint, const char *movement) {
    size_t current = lau_get_active_count();
    if (current > checkpoint) {
        fprintf(stderr, "\n[BALLET LEAK] %s: Found %zu leak(s)!\n", movement, current - checkpoint);
        lau_report_memory_metrics();
        // In a real performance, a leak is a fracture. We abort.
        abort();
    } else if (current < checkpoint) {
        fprintf(stderr, "\n[BALLET ANOMALY] %s: Active count %zu is LESS than checkpoint %zu!\n", movement, current, checkpoint);
    }
}

/**
 * TSFi Ballet Perfection: Formal declaration of the zero-leak foundation.
 * Must be executed to verify the manifold is clean before completing a movement.
 */
static inline void ballet_declare_perfection(void) {
    size_t active = lau_get_active_count();
    if (active == 0) {
        printf("\n[BALLET] Foundation Verified: PERFECTLY ZERO LEAKS\n");
    } else {
        fprintf(stderr, "\n[BALLET FRACTURE] Foundation has %zu active objects!\n", active);
        lau_report_memory_metrics();
        abort();
    }
}

#define BALLET_CHECKPOINT() lau_get_active_count()
#define BALLET_AUDIT(cp, movement) ballet_audit(cp, movement)
#define BALLET_DECLARE_PERFECTION() ballet_declare_perfection()

#endif // TSFI_BALLET_H
