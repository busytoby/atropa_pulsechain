#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_cla.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #14 Carry Look-Ahead (CLA) Test ===\n");

    TsfiAdderParams params = {
        .t_pd_and = 4.0f, // 4 ns
        .t_pd_or  = 4.0f, // 4 ns
        .t_pd_xor = 8.0f  // 8 ns
    };

    // Test 1: Verify correct logic for simple addition (5 + 3 = 8)
    uint8_t sum_rca = 0, sum_cla = 0;
    uint8_t cout_rca = 0, cout_cla = 0;
    float time_rca, time_cla;

    time_rca = tsfi_ripple_carry_adder(5, 3, 0, &sum_rca, &cout_rca, &params);
    time_cla = tsfi_carry_lookahead_adder(5, 3, 0, &sum_cla, &cout_cla, &params);

    printf("[CLA] 5 + 3 RCA -> Sum: %d, Cout: %d, Time: %f ns\n", sum_rca, cout_rca, time_rca);
    printf("[CLA] 5 + 3 CLA -> Sum: %d, Cout: %d, Time: %f ns\n", sum_cla, cout_cla, time_cla);
    assert(sum_rca == 8 && cout_rca == 0);
    assert(sum_cla == 8 && cout_cla == 0);

    // Test 2: Worst-case carry propagation (15 + 1)
    // 0b1111 + 0b0001 -> sum should be 0b0000, cout should be 1
    printf("[CLA] Worst-case addition: 15 + 1...\n");
    time_rca = tsfi_ripple_carry_adder(15, 1, 0, &sum_rca, &cout_rca, &params);
    time_cla = tsfi_carry_lookahead_adder(15, 1, 0, &sum_cla, &cout_cla, &params);

    printf("[CLA] 15 + 1 RCA -> Sum: %d, Cout: %d, Time: %f ns\n", sum_rca, cout_rca, time_rca);
    printf("[CLA] 15 + 1 CLA -> Sum: %d, Cout: %d, Time: %f ns\n", sum_cla, cout_cla, time_cla);

    assert(sum_rca == 0 && cout_rca == 1);
    assert(sum_cla == 0 && cout_cla == 1);

    // In a Ripple Carry Adder, the worst-case carry propagates sequentially through all 4 stages:
    // Stage 0 Carry out: P0 + C0 -> max(t_p0, t_c0) + t_pd_and + t_pd_or = 8 + 4 + 4 = 16ns
    // Stage 1 Carry out: P1 + C1 -> max(t_p1, t_c1) + t_pd_and + t_pd_or = max(8, 16) + 4 + 4 = 24ns
    // Stage 2 Carry out: P2 + C2 -> max(t_p2, t_c2) + t_pd_and + t_pd_or = max(8, 24) + 4 + 4 = 32ns
    // Stage 3 Carry out: P3 + C3 -> max(t_p3, t_c3) + t_pd_and + t_pd_or = max(8, 32) + 4 + 4 = 40ns
    // Sum 3 stabilizes at max(t_p3, t_c3) + t_pd_xor = max(8, 32) + 8 = 40ns
    //
    // In a CLA Adder:
    // Generate/Propagate stabilize at t_p = 8ns, t_g = 4ns
    // C4 stabilizes at max(t_g3, t_p3_g2, ...) + t_pd_or = max(4, 12, 12, 12, 12) + 4 = 16ns
    // Sum 3 stabilizes at max(t_p3, t_c3) + t_pd_xor = max(8, 16) + 8 = 24ns
    // Thus the CLA completes in 24ns, whereas the RCA completes in 40ns.
    assert(time_cla < time_rca);
    printf("[CLA] Verification passed: CLA settled significantly faster than RCA (%f ns vs %f ns).\n", time_cla, time_rca);

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_cla", NULL);

    printf("[PASS] Carry Look-Ahead (CLA) simulation verified.\n");
    return 0;
}
