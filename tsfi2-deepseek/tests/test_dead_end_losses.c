#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 10000000.0 // 10 MHz sampling rate to capture 1.6 MHz RF behavior
#define DURATION_SEC 0.005
#define NUM_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)

// Model of active grid tank coupled to ungrounded dead turns
typedef struct {
    double L_act;   // Active grid inductance (Henries)
    double C_tune;  // Tuning capacity (Farads)
    double L_dead;  // Floating dead-turns inductance (Henries)
    double C_s;     // Dead-turns stray capacity to ground (Farads)
    double M;       // Mutual inductance coupling (Henries)
    double R;       // Series resistance (Ohms)
    
    // State variables
    double i_act;   // Active coil current
    double vc_act;  // Active capacitor voltage (grid voltage)
    double i_dead;  // Dead-turns current
    double vc_dead; // Dead-turns capacitor voltage
} TsfiDeadEndCircuit;

static void circuit_process(
    TsfiDeadEndCircuit *ckt,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double dt
) {
    double L_act = ckt->L_act;
    double L_dead = ckt->L_dead;
    double M = ckt->M;
    double C_tune = ckt->C_tune;
    double C_s = ckt->C_s;
    double R = ckt->R;

    double i_act = ckt->i_act;
    double vc_act = ckt->vc_act;
    double i_dead = ckt->i_dead;
    double vc_dead = ckt->vc_dead;

    // Determinant of coupling matrix: det = L_act * L_dead - M * M
    double det = L_act * L_dead - M * M;
    assert(det > 0.0);

    for (size_t step = 0; step < count; step++) {
        double v_in = (double)input_rf[step];

        // Transient state equations for coupled loops:
        // L_act * d(i_act)/dt + M * d(i_dead)/dt = v_in - vc_act - i_act * R
        // M * d(i_act)/dt + L_dead * d(i_dead)/dt = -vc_dead - i_dead * R
        // Resolving derivatives using Cramer's rule:
        double v_act_loop = v_in - vc_act - i_act * R;
        double v_dead_loop = -vc_dead - i_dead * R;

        double di_act = (v_act_loop * L_dead - v_dead_loop * M) / det;
        double di_dead = (L_act * v_dead_loop - M * v_act_loop) / det;

        i_act += di_act * dt;
        i_dead += di_dead * dt;

        vc_act += (i_act / C_tune) * dt;
        vc_dead += (i_dead / C_s) * dt;

        output_grid[step] = (float)vc_act;
    }

    ckt->i_act = i_act;
    ckt->vc_act = vc_act;
    ckt->i_dead = i_dead;
    ckt->vc_dead = vc_dead;
}

int main() {
    printf("=== TSFi2 Paragon RA-6 Dead-End Losses Unit Test ===\n");

    // L_act = 120 uH, C_tune = 80 pF
    // L_dead = 80 uH, C_s = 120 pF (Dynamic self-resonance based on natural parameters)
    // R = 2.0 Ohm, M = 40 uH (High mutual coupling in close winding)
    TsfiDeadEndCircuit ckt_ungrounded;
    ckt_ungrounded.L_act = 120.0e-6;
    ckt_ungrounded.C_tune = 80.0e-12;
    ckt_ungrounded.L_dead = 80.0e-6;
    ckt_ungrounded.C_s = 120.0e-12;
    ckt_ungrounded.M = 40.0e-6; 
    ckt_ungrounded.R = 2.0;
    ckt_ungrounded.i_act = 0.0;
    ckt_ungrounded.vc_act = 0.0;
    ckt_ungrounded.i_dead = 0.0;
    ckt_ungrounded.vc_dead = 0.0;

    TsfiDeadEndCircuit ckt_grounded = ckt_ungrounded;
    ckt_grounded.M = 0.0;

    // Calculate natural self-resonant frequency dynamically
    double f_dead_theory = 1.0 / (2.0 * M_PI * sqrt(ckt_ungrounded.L_dead * ckt_ungrounded.C_s));
    printf("[THEORY] Dynamic Dead-Turn Resonant Spot: %.2f kHz\n", f_dead_theory / 1000.0);

    float *input_rf = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_un = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_gr = (float*)malloc(NUM_SAMPLES * sizeof(float));

    double dt = 1.0 / SAMPLING_RATE;

    // Sweep around the dynamic resonance center frequency
    double start_freq = f_dead_theory - 300000.0;
    double freq_span = 600000.0;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = start_freq + (freq_span / DURATION_SEC) * t;
        double phase = 2.0 * M_PI * freq * t;
        input_rf[i] = (float)sin(phase);
    }

    circuit_process(&ckt_ungrounded, input_rf, out_un, NUM_SAMPLES, dt);
    circuit_process(&ckt_grounded, input_rf, out_gr, NUM_SAMPLES, dt);

    // Verify grid signal reduction (dead spot absorption) near dynamic resonance
    double max_un_amp = 0.0;
    double max_gr_amp = 0.0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = start_freq + (freq_span / DURATION_SEC) * t;

        // Dynamic neighborhood match
        if (fabs(freq - f_dead_theory) < 50000.0) {
            double amp_un = fabs(out_un[i]);
            if (amp_un > max_un_amp) max_un_amp = amp_un;

            double amp_gr = fabs(out_gr[i]);
            if (amp_gr > max_gr_amp) max_gr_amp = amp_gr;
        }
    }

    printf("[TEST] Grid amplitude near dynamic spot (floating): %.4f V\n", max_un_amp);
    printf("[TEST] Grid amplitude near dynamic spot (grounded): %.4f V\n", max_gr_amp);

    // Grounded grid amplitude should be significantly higher due to lack of dead-turn absorption
    if (max_gr_amp > max_un_amp * 1.2) {
        printf("[SUCCESS] Dead-end switch parasitic losses and grounding recovery verified!\n");
    } else {
        printf("[FAILURE] Grounding recovery factor was insufficient.\n");
        free(input_rf);
        free(out_un);
        free(out_gr);
        return 1;
    }

    free(input_rf);
    free(out_un);
    free(out_gr);
    return 0;
}