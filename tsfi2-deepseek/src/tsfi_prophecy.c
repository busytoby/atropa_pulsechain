#include "tsfi_prophecy.h"
#include "tsfi_elektuur_issue25.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int tsfi_prophecy_generate_bank(const char *filepath, int grid_resolution) {
    FILE *f = fopen(filepath, "wb");
    if (!f) return -1;

    if (grid_resolution < 2) grid_resolution = 4;
    
    // Nested sweeps over key genes: 
    // gene 0 (R/frequency), gene 2 (gain), gene 4 (bias/crossover)
    for (int r = 0; r < grid_resolution; r++) {
        float val_r = (float)r / (grid_resolution - 1);
        for (int g = 0; g < grid_resolution; g++) {
            float val_g = (float)g / (grid_resolution - 1);
            for (int b = 0; b < grid_resolution; b++) {
                float val_b = (float)b / (grid_resolution - 1);

                TsfiBiotikaDna dna = {{val_r, 0.5f, val_g, 0.2f, val_b, 0.5f, 0.5f, 0.5f}};
                
                // 1. Simulate the Wien-Bridge oscillator
                TsfiWienBridgeOscillator osc;
                tsfi_wien_bridge_init(&osc, &dna);

                float buffer[500];
                float dt = 1e-5f;
                for (int step = 0; step < 500; step++) {
                    tsfi_wien_bridge_step(&osc, dt);
                    buffer[step] = osc.V_out;
                }

                // 2. Route the output through the Atoom-versterker
                TsfiAtoomVersterker amp;
                // Bias is modulated by gene 4 (val_b)
                tsfi_atoom_versterker_init(&amp, 25.0f);
                amp.bias_ntc = val_b * 0.4f; // Bias range [0V, 0.4V]

                float amplified[500];
                float peak = 0.0f;
                for (int step = 0; step < 500; step++) {
                    amplified[step] = tsfi_atoom_versterker_process(&amp, buffer[step]);
                    if (fabsf(amplified[step]) > peak) peak = fabsf(amplified[step]);
                }

                // Calculate metrics
                float crest = tsfi_biotika_calculate_crest_factor(amplified, 500);

                TsfiProphecyEntry entry;
                for (int i = 0; i < 8; i++) entry.genes[i] = dna.genes[i];
                entry.crest_factor = crest;
                entry.crossover_dist = amp.crossover_dist;
                entry.v_out_peak = peak;

                fwrite(&entry, sizeof(TsfiProphecyEntry), 1, f);
            }
        }
    }

    fclose(f);
    return 0;
}

int tsfi_prophecy_inverse_lookup(const char *filepath, float target_crest, float target_dist, TsfiBiotikaDna *out_dna) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;

    TsfiProphecyEntry entry;
    TsfiProphecyEntry best_entry;
    float min_distance = 1e18f;
    int found = 0;

    while (fread(&entry, sizeof(TsfiProphecyEntry), 1, f) == 1) {
        // Calculate Euclidean distance in feature space (crest factor & crossover distortion count)
        float d_crest = entry.crest_factor - target_crest;
        float d_dist = entry.crossover_dist - target_dist;
        float distance = sqrtf(d_crest * d_crest + d_dist * d_dist);

        if (distance < min_distance) {
            min_distance = distance;
            best_entry = entry;
            found = 1;
        }
    }

    fclose(f);

    if (found && out_dna) {
        for (int i = 0; i < 8; i++) {
            out_dna->genes[i] = best_entry.genes[i];
        }
        return 0;
    }

    return -2;
}
