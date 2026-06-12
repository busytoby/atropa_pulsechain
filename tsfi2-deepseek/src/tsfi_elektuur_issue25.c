#include "tsfi_elektuur_issue25.h"
#include <math.h>
#include <stdlib.h>

void tsfi_wien_bridge_init(TsfiWienBridgeOscillator *osc, const TsfiBiotikaDna *dna) {
    if (!osc) return;
    
    // Decode genes:
    // gene 0 maps R [1.0k, 100k]
    // gene 1 maps C [1nF, 10uF]
    // gene 2 maps base loop gain [2.5, 5.0]
    // gene 3 maps NTC time constant delta coeff [0.01, 0.5]
    
    float gene_r = dna ? dna->genes[0] : 0.5f;
    float gene_c = dna ? dna->genes[1] : 0.5f;
    float gene_gain = dna ? dna->genes[2] : 0.5f;
    
    osc->R1 = 1000.0f + gene_r * 99000.0f;
    osc->R2 = osc->R1;
    osc->C1 = 1e-9f + gene_c * 1e-5f;
    osc->C2 = osc->C1;
    
    osc->V_series = 0.1f; // Initial kickoff noise
    osc->V_parallel = 0.0f;
    osc->gain = 2.5f + gene_gain * 2.5f;
    osc->ntc_temp = 25.0f; // ambient
    osc->ntc_resistance = 10000.0f; // 10k at ambient
    osc->V_out = 0.0f;
}

void tsfi_wien_bridge_step(TsfiWienBridgeOscillator *osc, float dt) {
    if (!osc || dt <= 0.0f) return;

    // Series node current: I_s = (V_out - V_series - V_parallel) / R1
    float dV_series = ((osc->V_out - osc->V_series - osc->V_parallel) / (osc->R1 * osc->C1)) * dt;
    osc->V_series += dV_series;

    // Parallel node current: I_p = I_s - V_parallel / R2
    // Which yields: dV_parallel/dt = (V_series / (R1*C2) - V_parallel / (R2*C2))
    float dV_parallel = ((osc->V_series / (osc->R1 * osc->C2)) - (osc->V_parallel / (osc->R2 * osc->C2))) * dt;
    osc->V_parallel += dV_parallel;

    // Thermal NTC resistance feedback model:
    // Heat dissipation lowers NTC resistance, which stabilizes loop gain.
    float inst_power = osc->V_out * osc->V_out / osc->ntc_resistance;
    float cooling = 0.05f * (osc->ntc_temp - 25.0f);
    osc->ntc_temp += (inst_power - cooling) * dt * 10.0f; // time constant scale
    
    // NTC Resistance curve: R = R0 * e^(Beta * (1/T - 1/T0))
    // Approximate with linear slope for simple simulation stability
    osc->ntc_resistance = 10000.0f / (1.0f + 0.03f * (osc->ntc_temp - 25.0f));
    if (osc->ntc_resistance < 100.0f) osc->ntc_resistance = 100.0f;

    // Amplification with soft saturation (simulate Germanium saturation clip at 9.0V)
    float target_gain = osc->gain * (osc->ntc_resistance / 10000.0f);
    osc->V_out = tanhf(osc->V_parallel * target_gain) * 9.0f;
}

void tsfi_atoom_versterker_init(TsfiAtoomVersterker *amp, float temp) {
    if (!amp) return;
    amp->supply_voltage = 9.0f;
    amp->temp_celsius = temp;
    amp->input_gain = 10.0f;
    amp->bias_ntc = 0.22f; // optimal base-emitter threshold bias for Germanium
    amp->crossover_dist = 0.0f;
}

float tsfi_atoom_versterker_process(TsfiAtoomVersterker *amp, float input_val) {
    if (!amp) return 0.0f;

    // Germanium temperature leakage factor (doubles every 10C above 25C)
    float temp_factor = powf(2.0f, (amp->temp_celsius - 25.0f) / 10.0f);
    float icbo = 1e-6f * temp_factor; // base leakage current

    // Crossover distortion simulation: base-emitter forward voltage check
    // If the bias is below 0.2V (typical Germanium Vbe threshold), we get deadzones.
    float v_be_thresh = 0.2f + icbo * 1000.0f; // leakage increases effective Vbe threshold
    
    float gain_drive = input_val * amp->input_gain;
    float biased_input = gain_drive + amp->bias_ntc;

    float output = 0.0f;
    if (fabsf(biased_input) < v_be_thresh) {
        // Output sits in crossover dead-zone
        output = 0.0f;
        amp->crossover_dist += 1.0f;
    } else {
        // Normal amplification
        output = biased_input - (biased_input > 0 ? v_be_thresh : -v_be_thresh);
    }

    // Soft ceiling saturation
    if (output > amp->supply_voltage) output = amp->supply_voltage;
    if (output < -amp->supply_voltage) output = -amp->supply_voltage;

    return output;
}

float tsfi_biotika_calculate_crest_factor(const float *buffer, size_t size) {
    if (!buffer || size == 0) return 0.0f;
    float peak = 0.0f;
    float sum_sq = 0.0f;
    for (size_t i = 0; i < size; i++) {
        float val = fabsf(buffer[i]);
        if (val > peak) peak = val;
        sum_sq += val * val;
    }
    float rms = sqrtf(sum_sq / size);
    if (rms < 1e-5f) return 0.0f;
    return peak / rms;
}

TsfiBiotikaDna tsfi_biotika_crossover(const TsfiBiotikaDna *parent_a, const TsfiBiotikaDna *parent_b) {
    TsfiBiotikaDna child;
    for (int i = 0; i < 8; i++) {
        float blend = (float)rand() / (float)RAND_MAX;
        child.genes[i] = blend * parent_a->genes[i] + (1.0f - blend) * parent_b->genes[i];
    }
    return child;
}

void tsfi_biotika_mutate(TsfiBiotikaDna *dna, float mutation_rate) {
    if (!dna) return;
    for (int i = 0; i < 8; i++) {
        float roll = (float)rand() / (float)RAND_MAX;
        if (roll < mutation_rate) {
            float noise = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.1f;
            dna->genes[i] += noise;
            if (dna->genes[i] < 0.0f) dna->genes[i] = 0.0f;
            if (dna->genes[i] > 1.0f) dna->genes[i] = 1.0f;
        }
    }
}
