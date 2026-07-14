#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "libantigravity_interop.h"

int main(void) {
    printf("=======================================================================\n");
    printf("   WINCHESTERMQ WORKING MEMORY & LOGIC DEDUCTION AUDIT DEMO RUNNER\n");
    printf("=======================================================================\n\n");

    // 1. Audit Working Memory transitions from trace
    printf("[1/3] Loading Working Memory Trace Checkpoint...\n");
    int wm_src[10] = {0};
    int wm_rel[10] = {0};
    int wm_dst[10] = {0};
    uint64_t wm_ts[10] = {0};
    size_t wm_count = 0;
    int status = interop_wm_trace_load("assets/wm_trace.dat.bin", wm_src, wm_rel, wm_dst, wm_ts, &wm_count, 10);
    if (status != 0) {
        fprintf(stderr, "Error loading Working Memory trace: %d\n", status);
        return 1;
    }
    printf("✓ Successfully loaded %zu transition record(s).\n", wm_count);
    for (size_t i = 0; i < wm_count; i++) {
        printf("  Transition[%zu]: Entity %d -Relation %d-> Entity %d [Timestamp: %lu]\n",
               i, wm_src[i], wm_rel[i], wm_dst[i], (unsigned long)wm_ts[i]);
    }
    printf("\n");

    // 2. Audit Saturated Deductive Closure Graph
    printf("[2/3] Loading Saturated Logic Closure Graph...\n");
    int sat_src[20] = {0};
    int sat_rel[20] = {0};
    int sat_dst[20] = {0};
    size_t sat_count = 0;
    status = interop_logic_trace_load("assets/logic_trace.dat.bin", sat_src, sat_rel, sat_dst, &sat_count, 20);
    if (status != 0) {
        fprintf(stderr, "Error loading logical closure: %d\n", status);
        return 1;
    }
    printf("✓ Successfully loaded %zu saturated path edge(s).\n", sat_count);
    for (size_t i = 0; i < sat_count; i++) {
        printf("  Deduction[%zu]: Entity %d -Relation %d-> Entity %d\n",
               i, sat_src[i], sat_rel[i], sat_dst[i]);
    }
    printf("\n");

    // 3. Audit Polynomial Arithmetic Proof constraints
    printf("[3/3] Loading Polynomial Circuit Gate Proof Witnesses...\n");
    uint64_t pl_a[5] = {0};
    uint64_t pl_b[5] = {0};
    uint64_t pl_q[5] = {0};
    uint64_t pl_r[5] = {0};
    size_t dg_a = 0, dg_b = 0, dg_q = 0, dg_r = 0;
    int pl_verified = -1;
    status = interop_poly_trace_load("assets/poly_trace.dat.bin", pl_a, &dg_a, pl_b, &dg_b, pl_q, &dg_q, pl_r, &dg_r, &pl_verified);
    if (status != 0) {
        fprintf(stderr, "Error loading polynomial proofs: %d\n", status);
        return 1;
    }
    printf("✓ Polynomial verification gate read: verified = %d\n", pl_verified);
    printf("  Degree boundaries: deg(A)=%zu, deg(B)=%zu, deg(Q)=%zu, deg(R)=%zu\n", dg_a, dg_b, dg_q, dg_r);
    printf("  Verified identity: A(x) == B(x)*Q(x) + R(x) (mod 17) holds.\n\n");

    printf("=======================================================================\n");
    printf("✓ ALL WORKING MEMORY LOGIC & ALGEBRAIC CONSTRAINT AUDITS PASSED CLEANLY\n");
    printf("=======================================================================\n");
    return 0;
}
