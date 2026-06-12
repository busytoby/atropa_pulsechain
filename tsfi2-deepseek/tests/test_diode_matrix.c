#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_diode_matrix.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #19 Diode Matrix Decoder Test ===\n");

    TsfiDiodeMatrix matrix;
    tsfi_diode_matrix_init(&matrix);

    // Test 1: Decode logic correctness at 25C
    printf("[DECODER] Testing address decoding at 25C...\n");
    float dt = 10.0f; // 10 ns steps
    for (int addr = 0; addr < 8; addr++) {
        int selected = tsfi_diode_matrix_step(&matrix, addr, dt);
        printf("[DECODER]   Addr: %d -> Selected Output Line: %d (V_out: %f V)\n", addr, selected, matrix.v_outputs[selected]);
        
        assert(selected == addr);
        // Selected line should be high
        assert(matrix.v_outputs[selected] > 4.5f);
        
        // Unselected lines should be low (~0.9V)
        for (int j = 0; j < 8; j++) {
            if (j != selected) {
                assert(matrix.v_outputs[j] < 1.1f);
            }
        }
    }

    // Test 2: Thermal degradation test (115C)
    printf("[DECODER] Testing address decoding under thermal stress (115C)...\n");
    matrix.temp = 115.0f;

    // Run address 3 conversion
    int selected_115 = tsfi_diode_matrix_step(&matrix, 3, dt);
    float v_selected_115 = matrix.v_outputs[selected_115];
    printf("[DECODER]   115C Selected Line 3 V_out: %f V\n", v_selected_115);

    // At 115C, reverse leakage current has grown enough to drag the selected output down
    // from 5V to below 2.5V, degrading the logic 1 state.
    assert(v_selected_115 < 2.5f);
    printf("[DECODER]   Verification passed: High temperature leakage successfully degraded unselected noise margins.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_diode_matrix", NULL);

    printf("[PASS] Diode Matrix Decoder simulation verified.\n");
    return 0;
}
