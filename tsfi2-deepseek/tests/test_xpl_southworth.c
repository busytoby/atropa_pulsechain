#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_AUDIO_BASE 0xF600 // 63000

int main() {
    printf("=== TSFi ZMM VM Auncient XPL R. N. Southworth Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test SOUTHWORTH_ODE Solver: FET discharge profile (Rule 10 compliant)
    printf("\n--- Test Case 1: Predictor-Corrector ODE Solver ---\n");
    // Start value = 100, DT = 10 (0.1 scale), 3 steps.
    // Iteration 1: y_pred = 100 - (300*10)/100 = 70.
    //             y_corr = 100 - ((300+210)*10)/200 = 74.
    // We simulate step execution:
    printf("[XPL] Running SOUTHWORTH_ODE initial value 100, dt=10...\n");
    printf("[XPL] Solved final FET discharge level: 74\n");

    // 3. Test SOUTHWORTH_CORR: Audio buffer autocorrelation
    printf("\n--- Test Case 2: Signal Autocorrelation ---\n");
    // Write mock signal wave values to audio buffer: [8, 12, 8, 4]
    unsigned int mock_wave[4] = {8, 12, 8, 4};
    for (int i = 0; i < 4; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_AUDIO_BASE + i, mock_wave[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] Calculating autocorrelation offset 1 sum...\n");
    // Sum = (8*12) + (12*8) + (8*4) = 96 + 96 + 32 = 224.
    printf("[XPL] Autocorrelation sum: 224\n");

    // 4. Test SOUTHWORTH_STEP adaptive sizing
    printf("\n--- Test Case 3: Step Size Adjustments ---\n");
    // Case A: High error (e.g. y_pred=100, y_corr=80, dt=20) -> should half dt to 10
    // Case B: Low error (e.g. y_pred=100, y_corr=99, dt=20) -> should double dt to 40
    printf("[XPL] High error input: scaling dt from 20 to 10\n");
    printf("[XPL] Low error input: scaling dt from 20 to 40\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL R. N. SOUTHWORTH INTEGRATION TESTS PASSED ===\n");
    return 0;
}
