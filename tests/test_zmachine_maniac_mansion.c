#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "tsfi_montecarlo.h"
#include "tsfi_lnr_solvers.h"

void *lau_malloc_wired_loc(size_t size, const char *file, int line) {
    (void)file; (void)line;
    return malloc(size);
}

void *lau_memalign_loc(size_t alignment, size_t size, const char *file, int line) {
    (void)alignment; (void)file; (void)line;
    return malloc(size);
}

void lau_free(void *ptr) {
    free(ptr);
}

int tsfi_zorse_query_llm(const char *prompt, char *response_out, int max_len) {
    (void)prompt;
    if (response_out && max_len > 0) {
        response_out[0] = '\0';
    }
    return 0;
}

int main(void) {
    printf("=== Z-Machine Maniac Mansion Monte Carlo Renderer Test ===\n");

    // 1. Test the five solver thunks individually
    double A[16] = {
        4.0, 1.0, 0.0, 0.0,
        1.0, 5.0, 1.0, 0.0,
        0.0, 1.0, 3.0, 1.0,
        0.0, 0.0, 1.0, 4.0
    };
    double B[4] = { 5.0, 7.0, 5.0, 5.0 }; // Solution is [1, 1, 1, 1]
    double x[4] = {0};

    // Test LU
    memset(x, 0, sizeof(x));
    assert(lnr_solve_lu(A, B, x, 4));
    assert(fabs(x[0] - 1.0) < 1e-5);
    printf("   ✓ LU solver thunk passed\n");

    // Test QR
    memset(x, 0, sizeof(x));
    assert(lnr_solve_qr(A, B, x, 4));
    assert(fabs(x[0] - 1.0) < 1e-5);
    printf("   ✓ QR solver thunk passed\n");

    // Test SVD (Orthogonal QR decomposition iteration solver)
    memset(x, 0, sizeof(x));
    assert(lnr_solve_svd(A, B, x, 4));
    assert(fabs(x[0] - 1.0) < 1e-5);
    printf("   ✓ SVD solver thunk passed\n");

    // Test CG
    memset(x, 0, sizeof(x));
    assert(lnr_solve_cg(A, B, x, 4));
    assert(fabs(x[0] - 1.0) < 1e-5);
    printf("   ✓ CG solver thunk passed\n");

    // Test GMRES
    memset(x, 0, sizeof(x));
    assert(lnr_solve_gmres(A, B, x, 4));
    assert(fabs(x[0] - 1.0) < 1e-5);
    printf("   ✓ GMRES solver thunk passed\n");

    uint32_t *pixels = malloc(sizeof(uint32_t) * 64 * 64);
    float *depth = malloc(sizeof(float) * 64 * 64);
    assert(pixels != NULL);
    assert(depth != NULL);

    // Render a standard room frame (room 10)
    bool rc1 = tsfi_montecarlo_render_maniac_mansion(10, pixels, depth, 64, 64, 0.0f);
    assert(rc1 == true);
    printf("   ✓ Standard room render completed successfully\n");

    // Render a spooky room frame (room 13) which triggers regional fear
    bool rc2 = tsfi_montecarlo_render_maniac_mansion(13, pixels, depth, 64, 64, 0.5f);
    assert(rc2 == true);
    printf("   ✓ Spooky room (regional fear) render completed successfully\n");

    free(pixels);
    free(depth);

    printf("=== ALL MANIAC MANSION RENDERER TESTS PASSED ===\n");
    return 0;
}
