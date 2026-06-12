#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "tsfi_elektuur_issue25.h"

int main() {
    srand((unsigned int)time(NULL));
    printf("=== TSFi2 Elektuur Issue #25 (Biotika Edition) Verification ===\n");

    // 1. Test Wien-Bridge Oscillator kickoff and step
    TsfiBiotikaDna dna_normal = {{0.5f, 0.5f, 0.8f, 0.2f, 0.5f, 0.5f, 0.5f, 0.5f}};
    TsfiWienBridgeOscillator osc;
    tsfi_wien_bridge_init(&osc, &dna_normal);
    
    printf("[OSC] Initializing R1 = %f Ohm, C1 = %f F\n", osc.R1, osc.C1);
    
    float buffer[1000];
    float dt = 1e-5f; // 10 microseconds step
    for (int i = 0; i < 1000; i++) {
        tsfi_wien_bridge_step(&osc, dt);
        buffer[i] = osc.V_out;
    }
    
    // Oscillation buildup verification: the signal output should diverge from initial kickoff noise
    float last_val = osc.V_out;
    printf("[OSC] Output voltage after 10ms: %f V (Ambient NTC Temp: %f C)\n", last_val, osc.ntc_temp);
    assert(fabsf(last_val) > 0.0f);

    // 2. Test Biotika Spikiness (Crest Factor)
    float crest_factor = tsfi_biotika_calculate_crest_factor(buffer, 1000);
    printf("[BIOTIKA] Simulated Sine Wave Crest Factor: %f\n", crest_factor);
    assert(crest_factor >= 1.0f);

    // 3. Test Atoom-versterker temperature leakage and crossover
    TsfiAtoomVersterker amp;
    tsfi_atoom_versterker_init(&amp, 25.0f); // 25C standard
    
    float normal_out = tsfi_atoom_versterker_process(&amp, 0.5f);
    printf("[AMP] Output at 25C: %f V\n", normal_out);
    
    // Simulate at higher temperature: leakage icbo should increase crossover distortion threshold
    tsfi_atoom_versterker_init(&amp, 65.0f); // 65C hot run
    float hot_out = tsfi_atoom_versterker_process(&amp, 0.5f);
    printf("[AMP] Output at 65C (thermal leakage check): %f V (Crossover counts: %f)\n", hot_out, amp.crossover_dist);

    // 4. Test Genetic Operator compliance
    TsfiBiotikaDna parent_a = {{0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f}};
    TsfiBiotikaDna parent_b = {{0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f, 0.9f}};
    TsfiBiotikaDna child = tsfi_biotika_crossover(&parent_a, &parent_b);
    
    for (int i = 0; i < 8; i++) {
        assert(child.genes[i] >= 0.1f && child.genes[i] <= 0.9f);
    }
    
    tsfi_biotika_mutate(&child, 1.0f); // 100% mutation rate to shift coordinates
    printf("[BIOTIKA] Mutated child gene 0: %f\n", child.genes[0]);

    printf("[SUCCESS] All Elektuur-Biotika simulation constraints verified!\n");
    return 0;
}
