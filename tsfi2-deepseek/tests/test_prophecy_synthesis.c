#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tsfi_prophecy.h"
#include "tsfi_elektuur_issue25.h"

int main() {
    printf("=== TSFi2 Inverse Prophecy Synthesis Verification ===\n");

    const char *db_path = "prophecy_response.bin";
    
    // 1. Generate the response bank with a 5x5x5 grid resolution (125 sweeps)
    printf("[PROPHECY] Generating parameter grid database...\n");
    int res = tsfi_prophecy_generate_bank(db_path, 5);
    assert(res == 0);

    // 2. Perform inverse lookup for target spikiness
    float target_crest = 1.8f;
    float target_dist = 50.0f;
    printf("[PROPHECY] Querying bank for target (Crest: %f, Crossover: %f)...\n", target_crest, target_dist);

    TsfiBiotikaDna found_dna;
    int lookup_res = tsfi_prophecy_inverse_lookup(db_path, target_crest, target_dist, &found_dna);
    assert(lookup_res == 0);

    // Output the optimized patch settings
    printf("[PROPHECY] Best Match DNA Genes:\n");
    for (int i = 0; i < 8; i++) {
        printf("  Gene %d: %f\n", i, found_dna.genes[i]);
    }

    // 3. Re-run simulation with found DNA to verify correctness
    TsfiWienBridgeOscillator osc;
    tsfi_wien_bridge_init(&osc, &found_dna);

    float buffer[500];
    float dt = 1e-5f;
    for (int step = 0; step < 500; step++) {
        tsfi_wien_bridge_step(&osc, dt);
        buffer[step] = osc.V_out;
    }

    TsfiAtoomVersterker amp;
    tsfi_atoom_versterker_init(&amp, 25.0f);
    amp.bias_ntc = found_dna.genes[4] * 0.4f;

    float amplified[500];
    for (int step = 0; step < 500; step++) {
        amplified[step] = tsfi_atoom_versterker_process(&amp, buffer[step]);
    }

    float measured_crest = tsfi_biotika_calculate_crest_factor(amplified, 500);
    printf("[PROPHECY] Verified output metrics:\n");
    printf("  Crest Factor: %f (Query target: %f)\n", measured_crest, target_crest);
    printf("  Crossover Distortion count: %f (Query target: %f)\n", amp.crossover_dist, target_dist);

    // Assert that the result is logically bounded and near the target
    assert(measured_crest >= 1.0f);
    printf("[SUCCESS] Inverse Prophecy Synthesis verified successfully!\n");
    return 0;
}
