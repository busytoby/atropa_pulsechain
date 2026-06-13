#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 100000000.0 // 100 MHz sampling rate to resolve ns spark gap ionization
#define DURATION_SEC 0.00005       // 50 microseconds duration
#define NUM_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)

typedef struct {
    double C_bank;       // Condenser capacity (Farads)
    double L_helix;      // Primary helix inductance (Henries)
    double L_lead;       // Parasitic lead inductance (Henries)
    double V_breakdown;  // Sparkover voltage threshold (Volts)
    double R_arc;        // Arc plasma resistance (Ohms)
    double R_insulated;  // Insulated open gap resistance (Ohms)
    double tau_ion;      // Ionization time constant (seconds)

    // State variables
    double vc;           // Condenser voltage (V)
    double current;      // Loop current (A)
    double t_arc;        // Time elapsed since arc ionization start (seconds)
    int is_ionized;      // Flag indicating if the spark gap is currently ionized
} TsfiSparkHelixLoop;

static void spark_helix_process(
    TsfiSparkHelixLoop *ckt,
    float *out_current,
    float *out_vc,
    size_t count,
    double dt
) {
    double C_bank = ckt->C_bank;
    double L_total = ckt->L_helix + ckt->L_lead;
    double V_breakdown = ckt->V_breakdown;
    double R_arc = ckt->R_arc;
    double R_insulated = ckt->R_insulated;
    double tau_ion = ckt->tau_ion;

    double vc = ckt->vc;
    double current = ckt->current;
    double t_arc = ckt->t_arc;
    int is_ionized = ckt->is_ionized;

    for (size_t step = 0; step < count; step++) {
        // Evaluate spark gap state transitions
        if (!is_ionized && fabs(vc) >= V_breakdown) {
            is_ionized = 1;
            t_arc = 0.0;
        }

        // Dynamic gap resistance based on plasma decay
        double r_gap = R_insulated;
        if (is_ionized) {
            r_gap = R_arc + (R_insulated - R_arc) * exp(-t_arc / tau_ion);
            t_arc += dt;

            // Extinguish arc if current decays below threshold (e.g. 0.05 Amps)
            if (fabs(current) < 0.05 && t_arc > (5.0 * tau_ion)) {
                is_ionized = 0;
            }
        }

        // State equations:
        // L_total * di/dt = -vc - current * r_gap
        // dvc/dt = current / C_bank
        double di = (-vc - current * r_gap) / L_total;
        current += di * dt;
        vc += (current / C_bank) * dt;

        out_current[step] = (float)current;
        out_vc[step] = (float)vc;
    }

    ckt->vc = vc;
    ckt->current = current;
    ckt->t_arc = t_arc;
    ckt->is_ionized = is_ionized;
}

int main() {
    printf("=== TSFi2 Dr. Radio Spark-Helix Loop Unit Test ===\n");

    // Initialize two transmitter configurations:
    // 1. Efficient setup (short, heavy leads): L_lead = 0.2 uH
    // 2. Inefficient setup (long, thin leads): L_lead = 2.5 uH
    TsfiSparkHelixLoop ckt_efficient;
    ckt_efficient.C_bank = 0.005e-6; // 0.005 uF
    ckt_efficient.L_helix = 10.0e-6; // 10 uH
    ckt_efficient.L_lead = 0.2e-6;   // 0.2 uH (low parasitic)
    ckt_efficient.V_breakdown = 15000.0;
    ckt_efficient.R_arc = 0.2;
    ckt_efficient.R_insulated = 10.0e6;
    ckt_efficient.tau_ion = 5.0e-9;
    ckt_efficient.vc = 16000.0; // pre-charged condenser above breakdown
    ckt_efficient.current = 0.0;
    ckt_efficient.t_arc = 0.0;
    ckt_efficient.is_ionized = 0;

    TsfiSparkHelixLoop ckt_parasitic = ckt_efficient;
    ckt_parasitic.L_lead = 2.5e-6;   // 2.5 uH (high parasitic)

    float *cur_eff = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *vc_eff = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *cur_par = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *vc_par = (float*)malloc(NUM_SAMPLES * sizeof(float));

    double dt = 1.0 / SAMPLING_RATE;

    spark_helix_process(&ckt_efficient, cur_eff, vc_eff, NUM_SAMPLES, dt);
    spark_helix_process(&ckt_parasitic, cur_par, vc_par, NUM_SAMPLES, dt);

    // Calculate dynamic resonance frequency of spark oscillation by counting zero crossings
    int zero_cross_eff = 0;
    int zero_cross_par = 0;

    for (int i = 1; i < NUM_SAMPLES; i++) {
        if (cur_eff[i-1] * cur_eff[i] < 0.0f) zero_cross_eff++;
        if (cur_par[i-1] * cur_par[i] < 0.0f) zero_cross_par++;
    }

    double f_eff = (zero_cross_eff / 2.0) / DURATION_SEC;
    double f_par = (zero_cross_par / 2.0) / DURATION_SEC;

    printf("[ANALYSIS] Efficient Helix Loop Resonant Freq: %.2f kHz\n", f_eff / 1000.0);
    printf("[ANALYSIS] Parasitic Helix Loop Resonant Freq: %.2f kHz\n", f_par / 1000.0);

    // Parasitic loop has larger total inductance (L_helix + L_lead), shifting resonance to lower frequency
    if (f_eff > f_par * 1.05) {
        printf("[SUCCESS] Spark-helix loop breakdown and lead parasitic frequency shift verified!\n");
    } else {
        printf("[FAILURE] Resonance shift was insufficient.\n");
        free(cur_eff); free(vc_eff); free(cur_par); free(vc_par);
        return 1;
    }

    free(cur_eff); free(vc_eff); free(cur_par); free(vc_par);
    return 0;
}
