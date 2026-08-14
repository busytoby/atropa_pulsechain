#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_helmholtz_ring.h"

static bool test_thunk_matmul(float *x, int dim, double resonance_k, void *user_ctx) {
    (void)resonance_k; (void)user_ctx;
    for (int i = 0; i < dim; i++) x[i] *= 1.10f;
    return true;
}

static bool test_thunk_willms_prec(float *x, int dim, double resonance_k, void *user_ctx) {
    (void)user_ctx;
    float norm = 0.0f;
    for (int i = 0; i < dim; i++) norm += x[i] * x[i];
    norm = sqrtf(norm / (float)dim + (float)(resonance_k * resonance_k)) + 1e-6f;
    for (int i = 0; i < dim; i++) x[i] /= norm;
    return true;
}

static bool test_thunk_crostic_gate(float *x, int dim, double resonance_k, void *user_ctx) {
    (void)resonance_k; (void)user_ctx;
    for (int i = 0; i < dim; i++) {
        if (x[i] < 0.0f) x[i] = fabsf(x[i]);
    }
    return true;
}

int main(void) {
    printf("================ TEST DYNAMIC HELMHOLTZ THUNK RING DOMAIN ================\n");

    tsfi_helmholtz_ring_domain_t *ring = tsfi_helmholtz_ring_create(0x1010, 3, 0.125);
    assert(ring != NULL);
    assert(ring->count == 3);

    bool r0 = tsfi_helmholtz_ring_register_thunk(ring, 0, test_thunk_matmul, 0x1111);
    bool r1 = tsfi_helmholtz_ring_register_thunk(ring, 1, test_thunk_willms_prec, 0x2222);
    bool r2 = tsfi_helmholtz_ring_register_thunk(ring, 2, test_thunk_crostic_gate, 0x3333);
    assert(r0 && r1 && r2);

    float x_state[16];
    float x_checkpoint[16];
    for (int i = 0; i < 16; i++) {
        x_state[i] = (float)(i + 1) * 0.5f;
        x_checkpoint[i] = x_state[i];
    }

    // Dispatch Sequential Mode
    bool ok_seq = tsfi_helmholtz_ring_dispatch(ring, x_state, 16, true, NULL);
    assert(ok_seq);
    printf("  [PASS] Sequential Thunk Dispatch executed successfully.\n");

    // Perform ACID Rollback Rewind
    bool ok_rollback = tsfi_helmholtz_ring_acid_rollback(ring, x_state, x_checkpoint, 16);
    assert(ok_rollback);
    for (int i = 0; i < 16; i++) {
        assert(fabsf(x_state[i] - x_checkpoint[i]) < 1e-5f);
    }
    printf("  [PASS] ACID Rollback Rewind verified.\n");

    // Dispatch Flexible Mode under Willms Symmetry
    bool ok_flex = tsfi_helmholtz_ring_dispatch(ring, x_state, 16, false, NULL);
    assert(ok_flex);
    printf("  [PASS] Flexible Willms SO(2) Symmetry Dispatch executed successfully.\n");

    // Willms Bifurcation Audit Check
    float bif_metric = 0.0f;
    bool ok_bif = tsfi_willms_bifurcation_audit(x_state, 16, &bif_metric);
    assert(ok_bif);
    printf("  [PASS] Willms-Gladwell Bifurcation Audit verified (metric = %.4f).\n", bif_metric);

    // Willms Cauchy Boundary Pair Check (Gamma_in / Gamma_out)
    bool ok_cauchy = tsfi_willms_cauchy_boundary_check(x_state, 16, 0, 0x1);
    assert(ok_cauchy);
    printf("  [PASS] Willms Cauchy Boundary Pair Check verified.\n");

    // Willms Hopf Boundary Normal Gradient Gate Check
    bool ok_hopf = tsfi_willms_hopf_boundary_gate(x_state, 16);
    assert(ok_hopf);
    printf("  [PASS] Willms Hopf Boundary Normal Gradient Gate verified.\n");

    tsfi_helmholtz_ring_destroy(ring);
    printf("==========================================================================\n");
    return 0;
}
