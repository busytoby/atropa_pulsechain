#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// SMF Record Definition
typedef struct {
    uint32_t seq;
    uint32_t excp_count;
    char dataset_name[44];
} smf_point_t;

// Binary Quadtree Node Structure
typedef struct {
    double min_x, max_x; // Bounding box for Sequence Number
    double min_y, max_y; // Bounding box for EXCP count
    uint32_t point_count;
    uint32_t child_offsets[4]; // 0 if leaf, else file offset of child node
} daf_quad_node_t;

// 1. RED ANALYZER Gating: Static validation of query bounding box
static bool red_analyzer_validate_query_bounds(
    double min_x, double max_x,
    double min_y, double max_y
) {
    if (min_x > max_x || min_y > max_y) {
        return false; // Malformed search envelope
    }
    // Gating check: restrict scans targeting sequence ranges in supervisor space (< 10)
    if (min_x < 10.0) {
        return false; // RED rail block
    }
    return true;
}

// 2. BLACK XPLSM Gating: Quadtree division depth overflow check
#define MAX_DEPTH 3
static bool black_xplsm_subdivide_quadtree(
    double min_x, double max_x,
    double min_y, double max_y,
    const smf_point_t *points,
    uint32_t count,
    uint32_t current_depth,
    uint32_t *interrupt_out
) {
    if (count <= 1) return true; // Termination criteria: leaf node reached

    // Check depth limits
    if (current_depth > MAX_DEPTH) {
        if (interrupt_out) *interrupt_out = 33; // Trap 33: Quadtree Division Depth Overflow
        return false; // BLACK rail block
    }

    // Split points into 4 quadrants
    double mid_x = (min_x + max_x) / 2.0;
    double mid_y = (min_y + max_y) / 2.0;

    uint32_t q_counts[4] = {0};
    for (uint32_t i = 0; i < count; i++) {
        double px = points[i].seq;
        double py = points[i].excp_count;

        if (px <= mid_x && py <= mid_y) q_counts[0]++;
        else if (px > mid_x && py <= mid_y) q_counts[1]++;
        else if (px <= mid_x && py > mid_y) q_counts[2]++;
        else q_counts[3]++;
    }

    // Recursively subdivide quadrants
    for (int q = 0; q < 4; q++) {
        if (q_counts[q] > 0) {
            // Allocate points in quadrant
            smf_point_t *q_points = malloc(q_counts[q] * sizeof(smf_point_t));
            uint32_t idx = 0;
            for (uint32_t i = 0; i < count; i++) {
                double px = points[i].seq;
                double py = points[i].excp_count;

                bool match = false;
                if (q == 0 && px <= mid_x && py <= mid_y) match = true;
                else if (q == 1 && px > mid_x && py <= mid_y) match = true;
                else if (q == 2 && px <= mid_x && py > mid_y) match = true;
                else if (q == 3 && px > mid_x && py > mid_y) match = true;

                if (match) {
                    q_points[idx++] = points[i];
                }
            }

            // Subdivide
            bool ok = black_xplsm_subdivide_quadtree(
                (q % 2 == 0) ? min_x : mid_x, (q % 2 == 0) ? mid_x : max_x,
                (q < 2) ? min_y : mid_y, (q < 2) ? mid_y : max_y,
                q_points, q_counts[q], current_depth + 1, interrupt_out
            );
            free(q_points);
            if (!ok) return false;
        }
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DAF QUADTREE AUDIT DATABASE VALIDATION SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail static query audits
    printf("[TEST] Testing query bounding box limits on RED rail...\n");
    assert(red_analyzer_validate_query_bounds(10.0, 100.0, 1.0, 50.0) == true);
    
    // Malformed query coordinates
    assert(red_analyzer_validate_query_bounds(100.0, 10.0, 1.0, 50.0) == false);
    
    // Attempted query into supervisor sequence range (< 10)
    assert(red_analyzer_validate_query_bounds(5.0, 20.0, 1.0, 50.0) == false);
    printf("   ✓ RED rail successfully checked search envelope coordinate ranges.\n");

    // Test 2: BLACK rail division depth checks
    printf("[TEST] Evaluating quadtree subdivision levels against BLACK rail...\n");
    smf_point_t mock_points[] = {
        { 15, 20, "SYS1.ISPSLIB" },
        { 25, 45, "SYS1.ISPPLIB" }
    };
    uint32_t trap_code = 0;

    // Normal subdivision within depth limits
    assert(black_xplsm_subdivide_quadtree(10, 100, 10, 100, mock_points, 2, 0, &trap_code) == true);

    // Overflow subdivision depth (start depth exceeds MAX_DEPTH = 3)
    trap_code = 0;
    assert(black_xplsm_subdivide_quadtree(10, 100, 10, 100, mock_points, 2, 4, &trap_code) == false);
    assert(trap_code == 33); // Trap 33: Depth overflow
    printf("   ✓ BLACK rail successfully trapped quadtree division depth overflow (Trap 33).\n");

    printf("=============================================================\n");
    printf("ALL DAF QUADTREE AUDIT DATABASE TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
