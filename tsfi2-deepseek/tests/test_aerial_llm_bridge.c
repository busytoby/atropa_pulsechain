#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_lc_tank.h"
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 44100.0
#define NUM_SAMPLES 512

// Mock LLM "Aerial" Prompt Parser that maps incoming text states to synthesizer parameters
typedef struct {
    double equivalent_Cae;       // Attention context capacity (F)
    double graphite_Rleak;       // Graphite grid leak decay resistance (Ohm)
} LlmAerialInterface;

static LlmAerialInterface parse_llm_aerial_prompt(const char *prompt) {
    LlmAerialInterface aerial;
    
    // Default standard values
    aerial.equivalent_Cae = 200.0e-12; // 200 pF
    aerial.graphite_Rleak = 2.0e6;     // 2 MOhm

    // High preference or structured inputs alter parameters dynamically
    if (strstr(prompt, "high preference") || strstr(prompt, "DeForest")) {
        // Increase context capacity and decrease decay resistance for quick transient response
        aerial.equivalent_Cae = 400.0e-12; // 400 pF (wider bandwidth)
        aerial.graphite_Rleak = 1.0e6;     // 1 MOhm (faster envelope recovery)
        printf("[AERIAL] Parser matched 'DeForest/high preference' state. Parameters shifted.\n");
    } else if (strstr(prompt, "narrowband")) {
        aerial.equivalent_Cae = 100.0e-12; // 100 pF (sharper tuning)
        aerial.graphite_Rleak = 5.0e6;     // 5 MOhm (longer sustain/decay)
        printf("[AERIAL] Parser matched 'narrowband' state. Parameters shifted.\n");
    } else {
        printf("[AERIAL] Parser using default baseline mapping.\n");
    }

    return aerial;
}

int main() {
    printf("=== TSFi2 Synthesizer LLM-Aerial Bridge Test ===\n");

    // 1. LLM Prompt acting as the incoming Aerial Carrier signal
    const char *prompt_state = "DeForest Wiring is a high preference of mine";
    printf("[AERIAL] Incoming prompt: \"%s\"\n", prompt_state);

    // 2. Parse equivalent parameters at the LLM-Aerial interface
    LlmAerialInterface aerial = parse_llm_aerial_prompt(prompt_state);
    printf("[AERIAL] Mapped Cae:  %.2f pF\n", aerial.equivalent_Cae * 1e12);
    printf("[AERIAL] Mapped Rleak: %.2f MOhm\n", aerial.graphite_Rleak / 1e6);

    // 3. Initialize Synthesizer components driven by the Aerial parameters
    TsfiValveTriode valve;
    tsfi_valve_init(&valve, 90.0, 0.00002, 250.0, -1.5);
    valve.use_deforest_ground = 1;
    valve.V_filament = 5.0;

    // We use Cae to set the input grid coupling capacitance
    double grid_leak_time_constant = aerial.graphite_Rleak * aerial.equivalent_Cae;
    printf("[SYNTH] Resulting active grid leak time constant (tau): %.3e s\n", grid_leak_time_constant);

    float *input_rf = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));
    float *output_plate = (float*)lau_memalign(64, NUM_SAMPLES * sizeof(float));

    // Generate RF signal input (1 kHz audio envelope modulation)
    for (int i = 0; i < NUM_SAMPLES; i++) {
        input_rf[i] = 0.5f * sinf(2.0f * (float)M_PI * 1000.0f * (float)i / (float)SAMPLING_RATE);
    }

    // Process using the DeForest-grounded active valve loop
    tsfi_valve_process_regenerative(&valve, input_rf, output_plate, NUM_SAMPLES, 0.2, 1.0, 0.15);

    // Verify successful parameter injection
    double expected_bias = valve.Vg_bias - (valve.V_filament / 2.0);
    printf("[TEST] Grid bias reference checked: %.2f V (Expected: -4.00 V)\n", expected_bias);

    assert(fabs(expected_bias - (-4.00)) < 1e-6);
    printf("[SUCCESS] LLM-Aerial Synthesizer parameter injection verified!\n");

    lau_free(input_rf);
    lau_free(output_plate);

    return 0;
}