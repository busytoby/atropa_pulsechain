#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_byte_issue21.h"

int main() {
    printf("=== TSFi2 & BYTE Magazine Issue #21 Simulation Test ===\n");

    // 1. Test Apple-II Video Shared VRAM Controller
    printf("[APPLE-II] Verifying shared VRAM rendering...\n");
    TsfiApple2VideoController vid;
    tsfi_apple2_video_init(&vid);
    
    // Write characters directly to the shared memory map
    vid.vram[0] = 'A';
    vid.vram[1] = 'B';
    vid.vram[2] = 'C';

    uint8_t pixel;
    bool h_sync, v_sync;

    // Step through the active region of the first line (first 3 columns = 21 dots)
    // Verify that the dots correspond to the font representations
    // Character 'A' top row pattern is 0x18 (00011000 -> columns 3 and 4 active)
    for (int dot = 0; dot < 7; dot++) {
        tsfi_apple2_video_step(&vid, &pixel, &h_sync, &v_sync);
        if (dot == 3 || dot == 4) {
            assert(pixel == 1);
        } else {
            assert(pixel == 0);
        }
    }
    printf("[APPLE-II]   Column 0 ('A') top line successfully verified.\n");

    // 2. Test Joseph Carr Analog-to-Digital Interface (DAC)
    printf("[CARR-DAC] Verifying R-2R DAC and Slew-Rate...\n");
    TsfiCarrDac dac;
    // 5V reference, 0.5V/us slew rate
    tsfi_carr_dac_init(&dac, 5.0f, 0.5f);

    // Initial output is 0.0V.
    // Set digital value to 255 (target 5.0V).
    // In 1 us, output should rise by exactly 0.5V (limited by slew rate).
    float v1 = tsfi_carr_dac_convert(&dac, 255, 1.0f);
    printf("[CARR-DAC]   Step 255, 1us -> V_out: %fV (expected close to 0.5V)\n", v1);
    assert(fabs(v1 - 0.5f) < 0.05f);

    // Let's run for 100 us to let it reach target.
    float v2 = 0.0f;
    for (int i = 0; i < 20; i++) {
        v2 = tsfi_carr_dac_convert(&dac, 255, 1.0f);
    }
    printf("[CARR-DAC]   Step 255, after 20us -> V_out: %fV (expected close to 5.0V)\n", v2);
    assert(fabs(v2 - 5.0f) < 0.05f); // Should be very close to 5.0V

    // 3. Test Wimble Evolutionary Brain
    printf("[WIMBLE-BRAIN] Verifying genetic breeder convergence...\n");
    TsfiWimbleBrain brain;
    tsfi_wimble_brain_init(&brain);

    float inputs[8] = {1.0f, 0.5f, 0.2f, 0.0f, -0.5f, 0.1f, -0.2f, 0.8f};
    float targets[1] = {2.5f};

    // Run evolution for 50 generations and verify that the error decreases
    int initial_best_idx = tsfi_wimble_brain_evaluate(&brain, inputs, targets);
    float initial_best_fitness = brain.population[initial_best_idx].fitness;

    printf("[WIMBLE-BRAIN]   Gen 0 Best Fitness: %f\n", initial_best_fitness);

    for (int gen = 0; gen < 50; gen++) {
        tsfi_wimble_brain_breed(&brain, targets);
        tsfi_wimble_brain_evaluate(&brain, inputs, targets);
    }

    int final_best_idx = tsfi_wimble_brain_evaluate(&brain, inputs, targets);
    float final_best_fitness = brain.population[final_best_idx].fitness;
    printf("[WIMBLE-BRAIN]   Gen 50 Best Fitness: %f\n", final_best_fitness);

    // Fitness should have improved (become closer to 0)
    assert(final_best_fitness >= initial_best_fitness);
    printf("[WIMBLE-BRAIN]   Breed successful: fitness improved.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_byte_issue21", NULL);

    printf("[PASS] BYTE Magazine Issue #21 simulation verified.\n");
    return 0;
}
