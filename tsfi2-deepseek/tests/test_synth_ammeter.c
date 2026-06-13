#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_ammeter.h"

int main() {
    printf("=== TSFi2 Synthesizer Yul CPU to Ammeter Test ===\n");

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize DrumSynthesizer.yul in EVM context
    printf("[TEST] Loading DrumSynthesizer Yul contract...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"drum\", \"../solidity/bin/drumSynthesizer.yul\", 0x8ff9a7885e7CB3C01D556e1bC4f9fb2A6e7Ac167");

    if (strstr(vm.output_buffer, "Error") != NULL) {
        printf("[TEST] Error loading DrumSynthesizer: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    printf("[TEST] Yul Drum Synthesizer Contract Registered Successfully.\n");

    // Initialize the ammeter
    TsfiHotWireAmmeter meter;
    tsfi_ammeter_init(&meter);

    // Prepare simulation parameters
    size_t num_samples = 1000;
    double sample_rate = 10000.0;
    float *input_rf = malloc(num_samples * sizeof(float));
    float *out_theta = malloc(num_samples * sizeof(float));

    // Pitch coefficient: 0.1 * 1e18, Decay coefficient: 0.05 * 1e18
    // Lower 128 bits: 0.1 * 1e18 = 100000000000000000 (0x16345785d8a0000)
    // Upper 128 bits: 0.05 * 1e18 = 50000000000000000 (0xb1a2bc2ec50000)
    // Packed value in hex: 0x000000000000000000b1a2bc2ec500000000000000000000016345785d8a0000
    // Method selector: 0x07a96d8c
    // Let's generate a trigger at sample 0, then let the system decay
    for (size_t i = 0; i < num_samples; i++) {
        char cmd[512];
        uint64_t trigger = (i == 0) ? 1 : 0;
        
        snprintf(cmd, sizeof(cmd), 
                 "YULEXEC \"drum\", \"07a96d8c"
                 "%064lx" // trigger
                 "000000000000000000b1a2bc2ec500000000000000000000016345785d8a0000" // packedVal
                 "\"", trigger);

        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        tsfi_zmm_vm_exec(&vm, cmd);

        // Parse result from hex output (value scaled by 1e18)
        double signal_val = 0.0;
        if (strlen(vm.output_buffer) >= 16) {
            char *endptr = NULL;
            // Decode the 64-character hex output as signed 256-bit (simplified to signed long)
            long long raw_val = strtoll(vm.output_buffer, &endptr, 16);
            signal_val = (double)raw_val / 1e18;
        }
        
        // Scale signal_val to simulate a current in Amperes (e.g. max ~4 Amps)
        input_rf[i] = (float)(signal_val * 4.0);
    }

    printf("[TEST] Generated %zu synthesizer samples successfully.\n", num_samples);

    // 2. Process the Yul CPU generated audio through the ammeter
    tsfi_ammeter_process(&meter, input_rf, out_theta, num_samples, sample_rate);

    // Verify peak deflection and thermal lag
    double max_theta = 0.0;
    size_t max_idx = 0;
    for (size_t i = 0; i < num_samples; i++) {
        if (out_theta[i] > max_theta) {
            max_theta = out_theta[i];
            max_idx = i;
        }
    }

    printf("[TEST] Peak Needle Deflection: %.4f rad achieved at sample %zu\n", max_theta, max_idx);
    printf("[TEST] Final Heater Temperature: %.2f K (ambient: %.2f K)\n", meter.T, meter.T_ambient);

    // Assert that the needle deflected significantly from the center
    if (max_theta > 0.01) {
        printf("[SUCCESS] Synthesizer output successfully verified on the ammeter!\n");
    } else {
        printf("[FAILURE] Ammeter failed to deflect from synthesizer output.\n");
        free(input_rf); free(out_theta);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    free(input_rf); free(out_theta);
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
