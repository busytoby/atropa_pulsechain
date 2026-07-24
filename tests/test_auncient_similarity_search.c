#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define FEATURE_DIM 4
#define DB_SIZE 3

typedef struct {
    uint32_t record_id;
    uint8_t features[FEATURE_DIM];
} database_record_t;

// -------------------------------------------------------------
// Content-Based Similarity Search (Lew style)
// -------------------------------------------------------------
// Computes Manhattan distance (L1 norm) between two feature vectors
uint32_t compute_manhattan_distance(const uint8_t *v1, const uint8_t *v2) {
    uint32_t distance = 0;
    for (int i = 0; i < FEATURE_DIM; i++) {
        distance += (uint32_t)abs((int)v1[i] - (int)v2[i]);
    }
    return distance;
}

// Scans database records and returns the ID of the nearest neighbor (1-NN)
uint32_t find_nearest_neighbor(const database_record_t *db, const uint8_t *query_vector) {
    uint32_t nearest_id = db[0].record_id;
    uint32_t min_distance = compute_manhattan_distance(db[0].features, query_vector);

    for (int i = 1; i < DB_SIZE; i++) {
        uint32_t dist = compute_manhattan_distance(db[i].features, query_vector);
        if (dist < min_distance) {
            min_distance = dist;
            nearest_id = db[i].record_id;
        }
    }

    return nearest_id;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PIRK-LEW MULTIMEDIA SIMILARITY SEARCH SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Database slice containing 3 records with 4D feature vectors (e.g. color/texture descriptors)
    const database_record_t database[DB_SIZE] = {
        { .record_id = 101, .features = { 10, 20, 30, 40 } },
        { .record_id = 102, .features = { 100, 110, 120, 130 } },
        { .record_id = 103, .features = { 50, 60, 70, 80 } }
    };

    // Query feature vector close to Record 103
    const uint8_t query_vector[FEATURE_DIM] = { 55, 58, 72, 79 };

    printf("[TEST] Querying nearest neighbor for vector [55, 58, 72, 79]...\n");
    fflush(stdout);

    uint32_t matched_id = find_nearest_neighbor(database, query_vector);

    printf("   Matched Record ID: %d (Expected: 103)\n", matched_id);
    fflush(stdout);

    assert(matched_id == 103);
    printf("   ✓ Nearest neighbor matched expected record ID 103.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("LEW SIMILARITY SEARCH TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
