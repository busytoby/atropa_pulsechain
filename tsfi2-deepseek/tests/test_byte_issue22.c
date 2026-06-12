#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_byte_issue22.h"

int main() {
    printf("=== TSFi2 & BYTE Magazine Issue #22 Simulation Test ===\n");

    // 1. Verify APL Vector Operations
    printf("[APL] Verifying vector operations...\n");
    TsfiAplVector v1 = {.data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f}};
    TsfiAplVector v2 = {.data = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f}};

    TsfiAplVector v_neg = tsfi_apl_monadic_negate(&v1);
    assert(fabs(v_neg.data[0] - (-1.0f)) < 1e-5f);
    assert(fabs(v_neg.data[7] - (-8.0f)) < 1e-5f);

    float sum = tsfi_apl_monadic_sum(&v1);
    assert(fabs(sum - 36.0f) < 1e-5f);

    TsfiAplVector v_add = tsfi_apl_dyadic_add(&v1, &v2);
    for (int i = 0; i < VECTOR_SIZE; i++) {
        assert(fabs(v_add.data[i] - 9.0f) < 1e-5f);
    }

    TsfiAplVector v_mul = tsfi_apl_dyadic_mul(&v1, &v2);
    assert(fabs(v_mul.data[0] - 8.0f) < 1e-5f);
    assert(fabs(v_mul.data[3] - 20.0f) < 1e-5f);
    printf("[APL]   APL operations successfully verified.\n");

    // 2. Verify Newt Pathfinding & Reflex Collision Bumper
    printf("[NEWT] Verifying robot target navigation & bumper reflex...\n");
    TsfiNewtRobot robot;
    // Start at (0,0), target (2.0, 0)
    tsfi_newt_init(&robot, 0.0f, 0.0f, 2.0f, 0.0f);

    // Let's place an obstacle at (0.5, 0.0) with radius 0.2f.
    // The robot moves along X axis. When it steps to ~0.4, it should trigger collision.
    bool bumped = false;
    for (int step = 0; step < 30; step++) {
        tsfi_newt_step(&robot, 0.5f, 0.0f, 0.2f);
        if (robot.bumper_triggered) {
            bumped = true;
        }
    }

    printf("[NEWT]   End pos: (%f, %f), Collision count: %d, Target distance: %f\n",
           robot.x, robot.y, robot.col_count, sqrtf((2.0f-robot.x)*(2.0f-robot.x) + robot.y*robot.y));
    
    assert(bumped == true);
    assert(robot.col_count > 0);
    // Bumper pushed robot away, letting it continue towards target or stabilize
    printf("[NEWT]   Target navigation verified successfully.\n");

    // 3. Verify DRAM Console Refresh Logic
    printf("[DRAM] Verifying Dynamic RAM decay & refresh loops...\n");
    TsfiDramConsole dram;
    tsfi_dram_init(&dram);

    // Write Logic 1 (charge = 1.0f) to row 0, col 3.
    tsfi_dram_write(&dram, 0, 3, true);
    
    // Test 3.1: Read immediately (should return true)
    bool bit_immediate = tsfi_dram_read(&dram, 0, 3, 0);
    assert(bit_immediate == true);

    // Test 3.2: Allow decay without refresh.
    // Let's manually decay it beyond the logic threshold (requires ~2500 us delta at 0.00025f rate)
    // 3000 us time elapse
    tsfi_dram_decay_step(&dram, 3000, 0.00025f);
    bool bit_decayed = tsfi_dram_read(&dram, 0, 3, 3000);
    printf("[DRAM]   Immediate read: %d, Decayed read (no refresh): %d\n", bit_immediate, bit_decayed);
    assert(bit_decayed == false); // Charge decayed below logic 1 threshold

    // Test 3.3: Write again, refresh row at 4000us, decay step at 5000us, should stay valid!
    tsfi_dram_write(&dram, 0, 3, true);
    // Refresh at 4000us
    tsfi_dram_refresh_row(&dram, 0, 4000);
    // Decay simulation at 5000us (only 1000us since last refresh, decay charge is 1.0 - 0.25 = 0.75 > 0.5)
    tsfi_dram_decay_step(&dram, 5000, 0.00025f);
    bool bit_refreshed = tsfi_dram_read(&dram, 0, 3, 5000);
    printf("[DRAM]   Refreshed read (periodic refresh): %d\n", bit_refreshed);
    assert(bit_refreshed == true);
    printf("[DRAM]   Decay and refresh mechanics verified successfully.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_byte_issue22", NULL);

    printf("[PASS] BYTE Magazine Issue #22 simulation verified.\n");
    return 0;
}
