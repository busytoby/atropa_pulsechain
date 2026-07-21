#include "tsfi_parc_pulsechain_lp_matrix.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING PULSECHAIN LP POOL INGERMAN PATH MATRIX AUDIT \n");
    printf("=======================================================\n");

    tsfi_pulsechain_lp_matrix_t lp_matrix;
    assert(tsfi_pulsechain_lp_matrix_initialize(&lp_matrix) == 0);
    printf("[PASS] PulseChain LP Path Matrix Engine Initialized\n");

    // Register PulseChain Tokens
    uint32_t wpls, hex_tok, plsx, inc, atropa;
    assert(tsfi_pulsechain_lp_matrix_register_token(&lp_matrix, "WPLS", "dynamic_0xa1077a294d2b069e7002985b4638b008ba844001", &wpls) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_token(&lp_matrix, "HEX", "dynamic_0x2b591e99af9f32e18d36926474629911e3b5e408", &hex_tok) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_token(&lp_matrix, "PLSX", "dynamic_0x95b303987a60c5714464f88708b4081ef77f1545", &plsx) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_token(&lp_matrix, "INC", "dynamic_0x2fa878ab3f87417594368d1598f280b3967d66a0", &inc) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_token(&lp_matrix, "ATROPA", "dynamic_0xcc78a0ac01d4b666166b69a2d48074d284a1e944", &atropa) == 0);
    printf("[PASS] 5 PulseChain Tokens Registered (WPLS, HEX, PLSX, INC, ATROPA)\n");

    // Register LP Pools (Edges): WPLS-HEX, WPLS-PLSX, PLSX-INC, INC-ATROPA
    assert(tsfi_pulsechain_lp_matrix_register_pool(&lp_matrix, wpls, hex_tok, "dynamic_0xpool1", 1000000, 500000) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_pool(&lp_matrix, wpls, plsx, "dynamic_0xpool2", 2000000, 8000000) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_pool(&lp_matrix, plsx, inc, "dynamic_0xpool3", 4000000, 100000) == 0);
    assert(tsfi_pulsechain_lp_matrix_register_pool(&lp_matrix, inc, atropa, "dynamic_0xpool4", 50000, 300000) == 0);
    printf("[PASS] 4 PulseChain LP Pools Registered\n");

    // Compute Transitive Closure Closure Matrix via Ingerman Algorithm 141
    assert(tsfi_pulsechain_lp_matrix_compute_closure(&lp_matrix) == 0);
    printf("[PASS] Ingerman Algorithm 141 Transitive Closure Matrix Computed\n");

    // Verify Swap Multihop Reachability: HEX -> WPLS -> PLSX -> INC -> ATROPA
    assert(tsfi_pulsechain_lp_matrix_can_swap(&lp_matrix, hex_tok, atropa) == true);
    assert(tsfi_pulsechain_lp_matrix_can_swap(&lp_matrix, atropa, wpls) == true);
    assert(tsfi_pulsechain_lp_matrix_has_arbitrage_loop(&lp_matrix, wpls) == true);
    printf("[PASS] Multihop Reachability (HEX -> ATROPA: %s) & Arbitrage Loop Verified\n",
           tsfi_pulsechain_lp_matrix_can_swap(&lp_matrix, hex_tok, atropa) ? "TRUE" : "FALSE");

    char report[2048];
    assert(tsfi_pulsechain_lp_matrix_render_report(&lp_matrix, report, sizeof(report)) == 0);
    printf("\n%s\n", report);

    printf("=======================================================\n");
    printf(" ALL PULSECHAIN LP POOL INGERMAN MATRIX TESTS PASSED   \n");
    printf("=======================================================\n");

    return 0;
}
