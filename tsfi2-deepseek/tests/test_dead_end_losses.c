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

// Model of active grid tank coupled to multiple ungrounded dead-turn segments
typedef struct {
    double L_act;   // Active grid inductance (Henries)
    double C_tune;  // Tuning capacity (Farads)
    
    // Dead-turn segment 1
    double L_dead1; // Segment 1 inductance (Henries)
    double C_s1;    // Segment 1 stray capacity (Farads)
    double M1;      // Coupling to active coil (Henries)
    double R1;      // Resistance (Ohms)
    
    // Dead-turn segment 2
    double L_dead2; // Segment 2 inductance (Henries)
    double C_s2;    // Segment 2 stray capacity (Farads)
    double M2;      // Coupling to active coil (Henries)
    double R2;      // Resistance (Ohms)

    // Coupling between the two dead segments
    double M12;     // Mutual coupling between dead segments (Henries)

    double R_act;   // Active coil resistance (Ohms)
    
    // State variables
    double i_act;   // Active coil current
    double vc_act;  // Active capacitor voltage (grid voltage)
    double i_dead1; // Dead segment 1 current
    double vc_dead1;// Dead segment 1 capacitor voltage
    double i_dead2; // Dead segment 2 current
    double vc_dead2;// Dead segment 2 capacitor voltage
} TsfiDeadEndCircuit;

static void circuit_process(
    TsfiDeadEndCircuit *ckt,
    const float *input_rf,
    float *output_grid,
    size_t count,
    double dt
) {
    double L_act = ckt->L_act;
    double L_d1 = ckt->L_dead1;
    double L_d2 = ckt->L_dead2;
    double M1 = ckt->M1;
    double M2 = ckt->M2;
    double M12 = ckt->M12;
    double C_tune = ckt->C_tune;
    double C_s1 = ckt->C_s1;
    double C_s2 = ckt->C_s2;
    double R_act = ckt->R_act;
    double R1 = ckt->R1;
    double R2 = ckt->R2;

    double i_act = ckt->i_act;
    double vc_act = ckt->vc_act;
    double i_d1 = ckt->i_dead1;
    double vc_d1 = ckt->vc_dead1;
    double i_d2 = ckt->i_dead2;
    double vc_d2 = ckt->vc_dead2;

    // The system of coupled equations is:
    // [ L_act   M1      M2  ] [ di_act/dt ]   [ v_in - vc_act - i_act * R_act ]
    // [  M1    L_d1     M12 ] [  di_d1/dt ] = [ -vc_d1 - i_d1 * R1            ]
    // [  M2    M12     L_d2  ] [  di_d2/dt ]   [ -vc_d2 - i_d2 * R2            ]
    //
    // Let LHS matrix be A:
    // A_00 = L_act, A_01 = M1,    A_02 = M2
    // A_10 = M1,    A_11 = L_d1,  A_12 = M12
    // A_20 = M2,    A_21 = M12,   A_22 = L_d2

    double a00 = L_act, a01 = M1,   a02 = M2;
    double a10 = M1,    a11 = L_d1, a12 = M12;
    double a20 = M2,    a21 = M12,  a22 = L_d2;

    // Calculate determinant of 3x3 matrix using Sarrus rule:
    double det = a00 * (a11 * a22 - a12 * a21) 
               - a01 * (a10 * a22 - a12 * a20) 
               + a02 * (a10 * a21 - a11 * a20);
    assert(det > 0.0);

    for (size_t step = 0; step < count; step++) {
        double v_in = (double)input_rf[step];

        double b0 = v_in - vc_act - i_act * R_act;
        double b1 = -vc_d1 - i_d1 * R1;
        double b2 = -vc_d2 - i_d2 * R2;

        // Cramer's rule for 3x3:
        double det_act = b0 * (a11 * a22 - a12 * a21) 
                       - a01 * (b1 * a22 - a12 * b2) 
                       + a02 * (b1 * a21 - a11 * b2);

        double det_d1  = a00 * (b1 * a22 - a12 * b2) 
                       - b0 * (a10 * a22 - a12 * a20) 
                       + a02 * (a10 * b2 - b1 * a20);

        double det_d2  = a00 * (a11 * b2 - b1 * a21) 
                       - a01 * (a10 * b2 - b1 * a20) 
                       + b0 * (a10 * a21 - a11 * a20);

        double di_act = det_act / det;
        double di_d1  = det_d1 / det;
        double di_d2  = det_d2 / det;

        i_act += di_act * dt;
        i_d1  += di_d1 * dt;
        i_d2  += di_d2 * dt;

        vc_act += (i_act / C_tune) * dt;
        vc_d1  += (i_d1 / C_s1) * dt;
        vc_d2  += (i_d2 / C_s2) * dt;

        output_grid[step] = (float)vc_act;
    }

    ckt->i_act = i_act;
    ckt->vc_act = vc_act;
    ckt->i_dead1 = i_d1;
    ckt->vc_dead1 = vc_d1;
    ckt->i_dead2 = i_d2;
    ckt->vc_dead2 = vc_d2;
}

int main() {
    printf("=== TSFi2 Paragon RA-6 Multi-Segment Dead-End Losses Unit Test ===\n");

    // Initialize ungrounded circuit with two distinct dead-turn segments
    // producing separate absorption dips at different frequencies.
    TsfiDeadEndCircuit ckt_ungrounded;
    ckt_ungrounded.L_act = 120.0e-6;
    ckt_ungrounded.C_tune = 80.0e-12;
    
    // Segment 1 (Low-band dead spot)
    ckt_ungrounded.L_dead1 = 80.0e-6;
    ckt_ungrounded.C_s1 = 120.0e-12;
    ckt_ungrounded.M1 = 40.0e-6; 
    ckt_ungrounded.R1 = 2.0;

    // Segment 2 (High-band dead spot)
    ckt_ungrounded.L_dead2 = 40.0e-6;
    ckt_ungrounded.C_s2 = 60.0e-12;
    ckt_ungrounded.M2 = 20.0e-6;
    ckt_ungrounded.R2 = 3.0;

    // Coupling between components
    ckt_ungrounded.M12 = 10.0e-6;
    ckt_ungrounded.R_act = 2.0;

    ckt_ungrounded.i_act = 0.0;
    ckt_ungrounded.vc_act = 0.0;
    ckt_ungrounded.i_dead1 = 0.0;
    ckt_ungrounded.vc_dead1 = 0.0;
    ckt_ungrounded.i_dead2 = 0.0;
    ckt_ungrounded.vc_dead2 = 0.0;

    // Grounded equivalent (dead-ends shorted/decoupled)
    TsfiDeadEndCircuit ckt_grounded = ckt_ungrounded;
    ckt_grounded.M1 = 0.0;
    ckt_grounded.M2 = 0.0;
    ckt_grounded.M12 = 0.0;

    // Calculate theoretical resonant frequencies
    double f_dead1_theory = 1.0 / (2.0 * M_PI * sqrt(ckt_ungrounded.L_dead1 * ckt_ungrounded.C_s1));
    double f_dead2_theory = 1.0 / (2.0 * M_PI * sqrt(ckt_ungrounded.L_dead2 * ckt_ungrounded.C_s2));
    printf("[THEORY] Dead-Turn Segment 1 Resonant Spot: %.2f kHz\n", f_dead1_theory / 1000.0);
    printf("[THEORY] Dead-Turn Segment 2 Resonant Spot: %.2f kHz\n", f_dead2_theory / 1000.0);

    float *input_rf = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_un = (float*)malloc(NUM_SAMPLES * sizeof(float));
    float *out_gr = (float*)malloc(NUM_SAMPLES * sizeof(float));

    double dt = 1.0 / SAMPLING_RATE;

    // Wide sweep across both parasitic spots (from 1.0 MHz to 3.5 MHz)
    double start_freq = 1.0e6;
    double freq_span = 2.5e6;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = start_freq + (freq_span / DURATION_SEC) * t;
        double phase = 2.0 * M_PI * freq * t;
        input_rf[i] = (float)sin(phase);
    }

    circuit_process(&ckt_ungrounded, input_rf, out_un, NUM_SAMPLES, dt);
    circuit_process(&ckt_grounded, input_rf, out_gr, NUM_SAMPLES, dt);

    // Verify grid signal reduction near both resonant frequencies
    double max_un_amp1 = 0.0, max_gr_amp1 = 0.0;
    double max_un_amp2 = 0.0, max_gr_amp2 = 0.0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        double freq = start_freq + (freq_span / DURATION_SEC) * t;

        // Spot 1 neighborhood
        if (fabs(freq - f_dead1_theory) < 60000.0) {
            double amp_un = fabs(out_un[i]);
            if (amp_un > max_un_amp1) max_un_amp1 = amp_un;
            double amp_gr = fabs(out_gr[i]);
            if (amp_gr > max_gr_amp1) max_gr_amp1 = amp_gr;
        }

        // Spot 2 neighborhood
        if (fabs(freq - f_dead2_theory) < 60000.0) {
            double amp_un = fabs(out_un[i]);
            if (amp_un > max_un_amp2) max_un_amp2 = amp_un;
            double amp_gr = fabs(out_gr[i]);
            if (amp_gr > max_gr_amp2) max_gr_amp2 = amp_gr;
        }
    }

    printf("[TEST] Segment 1 spot grid amplitude (floating): %.4f V, (grounded): %.4f V\n", max_un_amp1, max_gr_amp1);
    printf("[TEST] Segment 2 spot grid amplitude (floating): %.4f V, (grounded): %.4f V\n", max_un_amp2, max_gr_amp2);

    int success = 1;
    if (max_gr_amp1 <= max_un_amp1 * 1.15) {
        printf("[FAILURE] Segment 1 grounding recovery factor was insufficient.\n");
        success = 0;
    }
    if (max_gr_amp2 <= max_un_amp2 * 1.15) {
        printf("[FAILURE] Segment 2 grounding recovery factor was insufficient.\n");
        success = 0;
    }

    if (success) {
        printf("[SUCCESS] Multi-segment dead-end switch parasitic losses and grounding recovery verified!\n");
    }

    free(input_rf);
    free(out_un);
    free(out_gr);
    return success ? 0 : 1;
}