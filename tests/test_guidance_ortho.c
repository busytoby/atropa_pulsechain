#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define VECTOR_DIM 8
#define CACHE_MAX_ENTRIES 4

// Structure representing cached orthonormal concept vector
typedef struct {
    char label[32];
    double vector[VECTOR_DIM];
    bool active;
} BasisCacheEntry;

static BasisCacheEntry g_basis_cache[CACHE_MAX_ENTRIES];

// Helper to compute dot product of two vectors
double dot_product(const double *v1, const double *v2, int dim) {
    double sum = 0.0;
    for (int i = 0; i < dim; i++) {
        sum += v1[i] * v2[i];
    }
    return sum;
}

// Helper to compute vector magnitude
double magnitude(const double *v, int dim) {
    return sqrt(dot_product(v, v, dim));
}

// Find entry in cache by label
BasisCacheEntry* find_cache_entry(const char *label) {
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (g_basis_cache[i].active && strcmp(g_basis_cache[i].label, label) == 0) {
            return &g_basis_cache[i];
        }
    }
    return NULL;
}

// Store a vector in the basis cache
void store_cache_entry(const char *label, const double *vector) {
    // Simple eviction policy: find first empty or overwrite
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (!g_basis_cache[i].active) {
            strncpy(g_basis_cache[i].label, label, sizeof(g_basis_cache[i].label) - 1);
            memcpy(g_basis_cache[i].vector, vector, VECTOR_DIM * sizeof(double));
            g_basis_cache[i].active = true;
            return;
        }
    }
}

// Orthonormalize and project an input embedding onto cached basis vectors
void project_and_cache(const char *style_label, const double *raw_style, const double *subject, double *out_residual) {
    double ortho_style[VECTOR_DIM];

    // Check if the style orthonormal basis is already built in memory
    BasisCacheEntry *cached = find_cache_entry(style_label);
    if (cached) {
        printf("[CACHE HIT] Resolved basis vector for style '%s' from memory\n", style_label);
        memcpy(ortho_style, cached->vector, VECTOR_DIM * sizeof(double));
    } else {
        printf("[CACHE MISS] Building and storing basis vector for style '%s'\n", style_label);
        
        // Gram-Schmidt step 1: Normalize raw style vector
        double mag = magnitude(raw_style, VECTOR_DIM);
        assert(mag > 1e-9);
        for (int i = 0; i < VECTOR_DIM; i++) {
            ortho_style[i] = raw_style[i] / mag;
        }
        
        store_cache_entry(style_label, ortho_style);
    }

    // Project subject vector onto the orthogonal complement of the style basis vector
    double projection = dot_product(subject, ortho_style, VECTOR_DIM);
    for (int i = 0; i < VECTOR_DIM; i++) {
        out_residual[i] = subject[i] - projection * ortho_style[i];
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EMBEDDING BASIS CACHE & GS-ORTHO TEST SUITE\n");
    printf("=============================================================\n");

    // Initialize cache empty
    memset(g_basis_cache, 0, sizeof(g_basis_cache));

    // Raw vectors representing different styles and subject prompts
    double raw_style_auncient[VECTOR_DIM] = { 0.88, -0.12, 0.33, 0.05, 0.44, -0.22, 0.15, -0.41 };
    double raw_subject_moth[VECTOR_DIM]    = { 0.55, -0.18, 0.32, 0.95, 0.12, 0.44, -0.22, 0.65 };

    double residual_1[VECTOR_DIM];
    double residual_2[VECTOR_DIM];

    // First run: Cache Miss. We compute and cache the orthonormal vector
    project_and_cache("auncient", raw_style_auncient, raw_subject_moth, residual_1);
    
    // Second run: Cache Hit. We load the vector from cache directly
    project_and_cache("auncient", raw_style_auncient, raw_subject_moth, residual_2);

    // Verify mathematical equivalency of both projection passes
    for (int i = 0; i < VECTOR_DIM; i++) {
        assert(fabs(residual_1[i] - residual_2[i]) < 1e-9);
    }
    printf("[TEST] Equivalency of cache results verified.\n");

    // Verify orthogonality of the residual complement against the style basis vector
    BasisCacheEntry *cached = find_cache_entry("auncient");
    assert(cached != NULL);
    double dot = dot_product(residual_1, cached->vector, VECTOR_DIM);
    printf("[TEST] Residual Orthogonality Dot Product: %.6e\n", dot);
    assert(fabs(dot) < 1e-9);

    printf("=============================================================\n");
    printf("ALL EMBEDDING BASIS CACHE TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
